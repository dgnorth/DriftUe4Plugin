"""Build and upload script to make UE4 client and server builds available on S3.
"""
import sys
import os
import threading
import time
import json
from datetime import datetime
import mimetypes
import argparse
import re
import getpass
import operator

from dateutil.parser import parse
from tabulate import tabulate
import boto3
from boto3.s3.transfer import S3Transfer, TransferConfig

def get_archives_in_folder(path):
    ret = []
    for filename in os.listdir(path):
        if filename.endswith(".zip"):
            full_filename = os.path.join(path, filename)
            ret.append(full_filename)
    return ret

def delete_archives(path):
    archives = get_archives_in_folder(path)
    for full_filename in archives:
        print "Deleting old archive '%s'" % full_filename
        os.remove(full_filename)

def get_script_path():
    return os.path.abspath(os.path.split(__file__)[0])

def get_config():
    config_filename = os.path.join(get_script_path(), "publish.cfg")
    ret = {}
    try:
        with open(config_filename, 'r') as f:
            ret = json.load(f)
    except:
        print "No config file. All configuration must come from command-line"
    return ret

def get_project_file():
    # assume the project file is one level above this script
    path = os.path.abspath(os.path.join(get_script_path(), "..\\")).replace('\\', '/')
    project_name = path.split('/')[-1]
    ret = os.path.join(path, project_name) + ".uproject"
    if not os.path.exists(ret):
        raise RuntimeError("Project file '%s' not found" % ret)
    return project_name, ret

config = get_config()

index_file = None

def get_index_path(s3path):
    return "{path}/index.json".format(path=s3path)

def get_index(s3region, s3bucket, s3path):
    global index_file
    if index_file:
        return index_file

    key_name = get_index_path(s3path)

    try:
        response = boto3.client('s3', s3region).get_object(Bucket=s3bucket, Key=key_name)
    except Exception as e:
        if 'NoSuchKey' not in str(e):
            raise
        index_file = {
            'repository': None,
            'next_build_number': 10000,
            'refs': [],
        }
    else:
        index_file = json.load(response['Body'])

    return index_file

def download_manifest(s3region, s3bucket, manifest_key_name):
    s3 = boto3.client('s3', s3region)
    resp = s3.get_object(Bucket=s3bucket, Key=manifest_key_name)
    ret = json.load(resp['Body'])
    return ret

def get_staging_directory(project_file, config):
    project_root, _ = os.path.split(project_file)
    project_root = os.path.abspath(project_root)
    return os.path.join(project_root, 'Saved', 'StagedBuilds', config)

def create_build_manifest(build_number, repository, ref, project, target_platform, config, version_string=None, executable_path=None):

    # Gather info for build manifest
    build_manifest = {
        'repository': repository,
        'ref': ref,
        'project': project,
        'target_platform': target_platform,
        'config': config,
        'timestamp': datetime.utcnow().isoformat(),
        'commit_id': None,
        'build_number': build_number,
        'built_by': getpass.getuser(),
        'version_string': version_string,
        'executable_path': executable_path,
    }

    # Generate a canonical name for the build archive (excluding extension)
    canonical_ref = '{}/{}'.format(ref, build_number).replace('/', '.')
    canonical_buildname = '{project}-{target_platform}-{config}-{canonical_ref}'.format(
        canonical_ref=canonical_ref, **build_manifest
        )
    build_manifest['build'] = canonical_buildname
    return build_manifest


def transfer_build_to_s3(archive_name, key_name):

    class ProgressPercentage(object):
        def __init__(self, filename):
            self._filename = filename
            self._size = float(os.path.getsize(filename))
            self._seen_so_far = 0
            self._lock = threading.Lock()
            self._start_time = time.time()
            self._last_time = time.time()
            self._megseg = 0.0

        @property
        def archive_info(self):
            return {
                "filename": self._filename,
                "size": long(self._size),
                "upload_time_sec": long(self._last_time - self._start_time)
            }

        def __call__(self, bytes_amount):
            # To simplify we'll assume this is hooked up
            # to a single filename.
            with self._lock:
                self._seen_so_far += bytes_amount
                percentage = (self._seen_so_far / self._size) * 100
                if time.time() - self._last_time > 0.02:
                    self._last_time = time.time()
                    elapsed = time.time() - self._start_time
                    self._megseg = (self._seen_so_far / 1024.0 / 1024.0) / elapsed
                    sys.stdout.write("Upload progress: %s kb / %s kb (%.2f%%) %.1f mb/s\r" % (self._seen_so_far // 1024, self._size // 1024, percentage, self._megseg))
                    sys.stdout.flush()

    transfer_config = TransferConfig(
        multipart_threshold=4 * 1024 * 1024,
        max_concurrency=30
    )

    client = boto3.client('s3', s3region)
    transfer = S3Transfer(client, transfer_config)
    mimetype, encoding = mimetypes.guess_type(archive_name)
    if mimetype is None:
        print "Can't figure out mimetype for:", archive_name
        sys.exit(1)

    print "    Archive filename:   ", archive_name
    print "    S3 Bucket:          ", s3bucket
    print "    S3 Key Name:        ", key_name
    print "    Key Mime Type:      ", mimetype

    cb = ProgressPercentage(archive_name)
    transfer.upload_file(
        archive_name, s3bucket, key_name,
        extra_args={'ContentType': mimetype},
        callback=cb,
        )
    
    return cb


def publish_build(zippathname, build_manifest, s3region, s3bucket, s3path):
    client = boto3.client('s3', s3region)

    # The archive and manifest must be stored in the correct subfolder, so we append
    # the UE4 build folder root and repository name.
    base_name = "{}/{}/{}".format(
        s3path,
        build_manifest['target_platform'], 
        build_manifest['build']
        )

    zipname, zipext = os.path.splitext(zippathname)
    archive_key_name = base_name + zipext
    manifest_key_name = base_name + '.json'

    # Upload the build to S3
    progress = transfer_build_to_s3(zippathname, archive_key_name)
    
    # Update manifest information
    build_manifest['archive_info'] = progress.archive_info
    build_manifest['archive'] = archive_key_name

    # Get a permalink to the build
    build_manifest['archive_url'] = client.generate_presigned_url(
        ClientMethod='get_object',
        Params={
            'Bucket': s3bucket,
            'Key': archive_key_name,
        },
        ExpiresIn=60*60*24*365,
        HttpMethod='GET'
    )

    # Upload build manifest. Use the same name as the archive, but .json.
    response = client.put_object(
        Bucket=s3bucket, 
        Key=manifest_key_name, 
        Body=json.dumps(build_manifest, indent=4), 
        ContentType='application/json'
    )
    print "build_manifest:", json.dumps(build_manifest, indent=4)

    # Index file update
    print "Updating index file"
    index_file = get_index(s3region, s3bucket, s3path)
    if index_file['next_build_number'] != build_manifest['build_number']:
        print "ATTENTION! Build number counter and build number don't match!"

    index_file['next_build_number'] += 1
    ref = build_manifest['ref']
    target_platform = build_manifest['target_platform']
    for ref_item in index_file['refs']:

        if ref_item['ref'] == ref and ref_item['target_platform'] == target_platform:
            break
    else:
        ref_item = {
            'ref': ref,
            'target_platform': target_platform,
            }
        index_file['refs'].append(ref_item)        

    # Add a reference to the manifest file
    ref_item['build_manifest'] = manifest_key_name

    key_name = get_index_path(s3path)
    response = client.put_object(
        Bucket=s3bucket, 
        Key=key_name, 
        Body=json.dumps(index_file, indent=4), 
        ContentType='application/json'
    )
    print "Publishing build succeeded"

def list_builds(s3region, s3bucket, s3path):
    sys.stdout.write('Fetching build information...')
    index_file = get_index(s3region, s3bucket, s3path)
    results = []
    for entry in index_file['refs']:
        sys.stdout.write('.')
        manifest = download_manifest(s3region, s3bucket, entry['build_manifest'])
        dt = manifest['timestamp']
        dt = parse(dt).replace(tzinfo=None).strftime("%Y-%m-%d %H:%M")
        sz = int(manifest['archive_info']['size'])/1024/1024
        results.append([entry['ref'], dt, manifest['built_by'], manifest['config'], entry['target_platform'], manifest['build_number'], sz])

    results.sort(key=operator.itemgetter(5), reverse=True)
    print
    print tabulate(results, headers=['ref', 'timestamp', 'built by', 'config', 'platform', 'build number', 'size [mb]'])

if __name__ == "__main__":
    start_time = time.time()
    parser = argparse.ArgumentParser()

    subparsers = parser.add_subparsers(help='sub-command help', dest="cmd")
    parser_list = subparsers.add_parser('list', help='List published builds')
    parser_list.add_argument('--s3bucket', default=config.get('bucket'), help="S3 Bucket name (default: %s)" % config.get('bucket'))
    parser_list.add_argument('--s3region', default=config.get('region'), help="S3 Region name (default: %s)" % config.get('region'))
    parser_list.add_argument('--s3path', default=config.get('path'), help="S3 Path (default: %s)" % config.get('path'))

    parser_publish = subparsers.add_parser('publish', help='Publish a build to the cloud')

    parser_publish.add_argument("-r", "--ref", required=True, help='Ref to publish this build under (required)')
    parser_publish.add_argument("-c", "--config", default="Development", help='Build configuration that was built (default: Development)')
    parser_publish.add_argument('-a', '--archive', help="Path to archive file to upload to S3. If not specified all .zip archives from staging folder will be published.")
    parser_publish.add_argument('-v', '--version-string', help="A canonical version string of the build (optional).")
    parser_publish.add_argument('-p', '--platform', default="Win64", help="Platform of the build (default: Win64)")

    parser_publish.add_argument('--s3bucket', default=config.get('bucket'), help="S3 Bucket name (default: %s)" % config.get('bucket'))
    parser_publish.add_argument('--s3region', default=config.get('region'), help="S3 Region name (default: %s)" % config.get('region'))
    parser_publish.add_argument('--s3path', default=config.get('path'), help="S3 Path (default: %s)" % config.get('path'))

    args = parser.parse_args()

    tp_archives = {}  # Key is target platform, value is archive folder, zip file name.
    build_manifests = []

    project_name, project_file = get_project_file()

    s3region = args.s3region
    s3bucket = args.s3bucket
    s3path = args.s3path
    if not all([s3region, s3bucket, s3path]):
        print "Missing required parameters. Please run command with --help for details"
        sys.exit(1)

    if args.cmd == 'publish':
        server_platform = args.platform
        executable_path = "{project_name}\\Binaries\\{server_platform}\\{project_name}Server.exe".format(project_name=project_name,
                                                                                                         server_platform=server_platform)

        config_name = args.config
        ref = args.ref
        REF_MAX_LEN = 16
        if len(ref) > REF_MAX_LEN:
            print "ref can be at most %s characters" % REF_MAX_LEN
            sys.exit(2)
        re1 = re.compile(r"[\w.-]*$")
        if not re1.match(ref):
            print "ref cannot contain any special characters other than . and -"
            sys.exit(2)

        if args.archive:
            archives = args.archive.split(",")
        else:
            staging_directory = get_staging_directory(project_file, config_name)
            archives = get_archives_in_folder(staging_directory)
            if len(archives) == 0:
                print "No archives found in folder '%s'. Nothing to publish!" % staging_directory
                sys.exit(2)

        index_file = get_index(s3region, s3bucket, s3path)

        for archive in archives:
            if not os.path.exists(archive):
                print "Archive '%s' not found. Cannot publish" % archive
                sys.exit(1)
        for archive in archives:
            target_platform = archive.replace("\\", ".").split(".")[-2]
            print "Publishing target platform '%s'" % target_platform

            build_manifest = create_build_manifest(
                build_number=index_file['next_build_number'],
                repository=s3path, 
                ref=args.ref,
                project=project_name, 
                target_platform=target_platform,
                config=config_name,
                version_string=args.version_string,
                executable_path=executable_path,
                )

            publish_build(archive, build_manifest, s3region, s3bucket, s3path)
            build_manifests.append(build_manifest)
        
        if build_manifests:
            print "Build manifests:"
            for build_manifest in build_manifests:
                print json.dumps(build_manifest, indent=4)
            print
            for build_manifest in build_manifests:
                print "Archive URL: %s" % build_manifest['archive_url']


    elif args.cmd == 'list':
        list_builds(s3region, s3bucket, s3path)
