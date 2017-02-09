"""Build and upload script to make UE4 client and server builds available on S3.
"""
import sys
import os
import threading
import time
import json
from datetime import datetime
import mimetypes
import subprocess
import shutil
import logging
import argparse
import glob
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
    config_filename = os.path.join(get_script_path(), "config.json")
    with open(config_filename, 'r') as f:
        ret = json.load(f)
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

def get_index_path():
    return "{path}/index.json".format(path=config['s3_path'])

def get_index():
    global index_file
    if index_file:
        return index_file

    key_name = get_index_path()

    try:
        response = boto3.client('s3', config['s3_region']).get_object(Bucket=config['s3_bucket'], Key=key_name)
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

def download_manifest(manifest_key_name):
    s3 = boto3.client('s3', config['s3_region'])
    resp = s3.get_object(Bucket=config['s3_bucket'], Key=manifest_key_name)
    ret = json.load(resp['Body'])
    return ret

# Build platform specifics
if sys.platform == 'win32':
    RUNUAT = 'RunUAT.bat'
elif sys.platform == 'darwin':
    RUNUAT = 'RunUAT.command'
elif sys.platform == 'linux':
    RUNUAT = 'RunUAT.sh'
else:
    raise RuntimeError("Build platform %s not supported.", sys.platform)

def get_staging_directory(project_file, config):
    project_root, _ = os.path.split(project_file)
    project_root = os.path.abspath(project_root)
    return os.path.join(project_root, 'Saved', 'StagedBuilds', config)


def get_ue4_root(project_name):
    """Figure out which UE4 engine source is associated with 'project_name'."""
    with open(project_name) as f:
        project = json.load(f)

    if sys.platform == 'win32':
        from _winreg import OpenKey, QueryValueEx, HKEY_CURRENT_USER
        key_name = r'SOFTWARE\Epic Games\Unreal Engine\Builds'
        key = OpenKey(HKEY_CURRENT_USER, key_name)
        try:
            ue4_root, reg_type = QueryValueEx(key, project['EngineAssociation'])
        except WindowsError as e:
            print "Can't figure out UE4 root. Please specify explicitly."
            print e
            print "Engine '%s' from project file was probably not found in registry key '%s'" % (project['EngineAssociation'], key_name)
            sys.exit(1)
    elif sys.platform == 'darwin':
        raise RuntimeError("Please implmement me!")
    else:
        raise RuntimeError("Build platform %s not supported.", sys.platform)

    return ue4_root


def build_project(project_file, config_name=None, server_platform=None, client_platform=None, map_name=None):
    """Build UE4 project.
    
    'ue4_root' is the root folder of the UE4 engine to use.
    'project_file' is the UE4 project to build.
    'config' is one of the UE4 project configs to use. Usually 'Shipping' or
    'Debug'.
    'server_platform' if set, builds a dedicated server for the platform specified.
    'client_platform' if set, builds a client for the platform specified.

    The function returns a pathname to the build artifact (normally a staged folder
    containing the image of the build).
    """
    if not config_name:
        raise RuntimeError("No build config specified.")

    if not client_platform and not server_platform:
        raise RuntimeError("build_project: 'server_platform' and 'client_platform' are None.")

    ue4_root = get_ue4_root(project_file)

    cmd = [
        os.path.join(ue4_root, 'Engine', 'Build', 'BatchFiles', RUNUAT),
        'BuildCookRun',
        '-project=' + os.path.abspath(project_file).replace('\\', '\\\\'),
        '-noP4',
        '-clientconfig=' + config_name,
        '-serverconfig=' + config_name,
        '-nocompileeditor',
        '-ue4exe=UE4Editor-Cmd.exe',
        '-cook',
        '-build',
        '-stage',
        '-map=' + (map_name or ""),
        '-pak',
        '-iterativecooking ',
        '-compressed',
        '-archive',
        '-stagingdirectory=%s' % get_staging_directory(project_file, config_name)
    ]

    '''
    -ScriptsForProject=G:/SuperKaijuVR/SuperKaijuVR.uproject BuildCookRun -project=G:/SuperKaijuVR/SuperKaijuVR.uproject -noP4 -clientconfig=Debug -serverconfig=Debug -nocompileeditor -ue4exe=UE4Editor-Cmd.exe -utf8output -server -serverplatform=Win64 -build -cook -map=Battle_Lava+End+Lobby+Login+Main -pak -iterativecooking -compressed -stage -deploy -cmdline= -Messaging -addcmdline=-SessionId=673189BA40E9E1EDA79BEB997307E3B2 -SessionOwner='matth' -SessionName='deditcatedad server' -run -compile
    '''

    platform = None
    if server_platform:
        cmd.append('-server')
        cmd.append('-serverplatform=' + server_platform)
        platform = server_platform

    if client_platform:
        platform = client_platform
    else:
        cmd.append('-noclient')
        
    cmd.append('-platform=' + platform)  # This parameter must always be set, because of reasons.

    print "Build commands:"
    print "    Project: ", project_file
    print "    Server:  ", server_platform
    print "    Client:  ", client_platform
    print "    Config:  ", config_name

    cmd = " ".join(cmd)
    print "Executing: ", cmd

    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    target_platforms = []

    while p.poll() is None:
        line = p.stdout.readline()
        sys.stdout.write(line)
        m = re.search('^.*-TargetPlatform=(.*?)\s', line)
        if m:
            tp = m.groups()[0]  # 'tp' is something like 'WindowsNoEditor+WindowsServer'
            target_platforms = tp.split('+')
    
    p.wait()

    if p.returncode != 0:
        print "Build failed with code ", p.returncode
        sys.exit(p.returncode)

    return target_platforms


def archive_build(image_folder):

    # Make archive name the same as the folder
    zippathname = os.path.join(image_folder)
    zippathname = os.path.abspath(zippathname)

    print "Creating archive"
    logging.basicConfig(level=logging.INFO)
    log = logging.getLogger("publisher")
    zippathname = shutil.make_archive(zippathname, 'zip', image_folder, logger=log)

    print "Archive ready: ", zippathname
    return zippathname


def create_build_manifest(build_number, repository, ref, project, target_platform, config, commit_id, version_string=None, executable_path=None):

    # Gather info for build manifest
    build_manifest = {
        'repository': repository,
        'ref': ref,
        'project': project,
        'target_platform': target_platform,
        'config': config,
        'timestamp': datetime.utcnow().isoformat(),
        'commit_id': commit_id,
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

    client = boto3.client('s3', config['s3_region'])
    transfer = S3Transfer(client, transfer_config)
    mimetype, encoding = mimetypes.guess_type(archive_name)
    if mimetype is None:
        print "Can't figure out mimetype for:", archive_name
        sys.exit(1)

    print "    Archive filename:   ", archive_name
    print "    S3 Bucket:          ", config['s3_bucket']
    print "    S3 Key Name:        ", key_name
    print "    Key Mime Type:      ", mimetype

    cb = ProgressPercentage(archive_name)
    transfer.upload_file(
        archive_name, config['s3_bucket'], key_name,
        extra_args={'ContentType': mimetype},
        callback=cb,
        )
    
    return cb


def publish_build(zippathname, build_manifest):
    client = boto3.client('s3', config['s3_region'])

    # The archive and manifest must be stored in the correct subfolder, so we append
    # the UE4 build folder root and repository name.
    base_name = "{}/{}/{}".format(
        config['s3_path'],
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
            'Bucket': config['s3_bucket'],
            'Key': archive_key_name,
        },
        ExpiresIn=60*60*24*365,
        HttpMethod='GET'
    )

    # Upload build manifest. Use the same name as the archive, but .json.
    response = client.put_object(
        Bucket=config['s3_bucket'], 
        Key=manifest_key_name, 
        Body=json.dumps(build_manifest, indent=4), 
        ContentType='application/json'
    )
    print "build_manifest:", json.dumps(build_manifest, indent=4)

    # Index file update
    print "Updating index file"
    index_file = get_index()
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

    key_name = get_index_path()
    response = client.put_object(
        Bucket=config['s3_bucket'], 
        Key=key_name, 
        Body=json.dumps(index_file, indent=4), 
        ContentType='application/json'
    )
    print "Publishing build succeeded"

def list_builds():
    sys.stdout.write('Fetching build information...')
    index_file = get_index()
    results = []
    for entry in index_file['refs']:
        sys.stdout.write('.')
        manifest = download_manifest(entry['build_manifest'])
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

    parser_build = subparsers.add_parser('build', help='Build the UE4 project')
    parser_build.add_argument('-c', '--client', action='store_true', help="Also build a client")

    parser_publish = subparsers.add_parser('publish', help='Publish a build to the cloud')

    parser_publish.add_argument("-r", "--ref", required=True, help='Ref to publish this build under')
    parser_publish.add_argument("-c", "--commit-id", required=False, help='Commit ID to tag the build with (optional)')
    parser_publish.add_argument('-a', '--archive', help="Path to archive file to upload to S3.")
    parser_publish.add_argument('-v', '--version-string', help="A canonical version string of the build.")

    args = parser.parse_args()

    tp_archives = {}  # Key is target platform, value is archive folder, zip file name.
    build_manifests = []

    project_name, project_file = get_project_file()
    config_name = config.get('config')
    server_platform = config.get('server_platform')
    executable_path = "{project_name}\\Binaries\\{server_platform}\\{project_name}Server.exe".format(project_name=project_name,
                                                                                                     server_platform=server_platform)

    if args.cmd == 'build':
        print "Building project %s..." % project_name
        map_name = config.get('map_name')
        client_platform = None
        if args.client:
            client_platform = config.get('client_platform')

        target_platforms = build_project(
            project_file=project_file,
            config_name=config_name,
            server_platform=server_platform,
            client_platform=client_platform,
            map_name=map_name
            )
        # Make a list of all the target build artifact folders
        tp_archives = {}
        for tp in target_platforms:
            staging_directory = get_staging_directory(project_file, config_name)
            archive_folder = os.path.join(staging_directory, tp)
            print "archive folder for platform '%s': %s" % (tp, archive_folder)
            delete_archives(staging_directory)
            tp_archives[tp] = {"archive_folder": archive_folder}

        for tp, archive_info in tp_archives.items():
            zipname = archive_build(archive_info["archive_folder"])
            archive_info["archive_filename"] = zipname

        diff = time.time() - start_time
        print "\nBuild completed in %.0fsec. The following archives are available for publishing:" % diff
        for info in tp_archives.values():
            print "    %s" % info['archive_filename'].replace("\\\\", "\\")
        print

    elif args.cmd == 'publish':
        ref = args.ref
        REF_MAX_LEN = 16
        if len(ref) > REF_MAX_LEN:
            print "ref can be at most %s characters" % REF_MAX_LEN
            sys.exit(2)
        re1 = re.compile(r"[\w.-]*$")
        if not re1.match(ref):
            print "ref cannot contain any special characters other than . and -"
            sys.exit(2)

        s3_path = config.get('s3_path')
        staging_directory = get_staging_directory(project_file, config_name)
        archives = get_archives_in_folder(staging_directory)
        if len(archives) == 0:
            print "No archives found in folder '%s'. Nothing to publish!" % staging_directory
            sys.exit(2)

        index_file = get_index()

        for archive in archives:
            target_platform = archive.replace("\\", ".").split(".")[-2]
            print "Publishing target platform '%s'" % target_platform

            build_manifest = create_build_manifest(
                build_number=index_file['next_build_number'],
                repository=s3_path, 
                ref=args.ref,
                project=project_name, 
                target_platform=target_platform,
                config=config_name,
                commit_id=args.commit_id,
                version_string=args.version_string,
                executable_path=executable_path,
                )

            publish_build(archive, build_manifest)
            build_manifests.append(build_manifest)
        
        if build_manifests:
            print "Build manifests:"
            for build_manifest in build_manifests:
                print json.dumps(build_manifest, indent=4)
            print
            for build_manifest in build_manifests:
                print "Archive URL: %s" % build_manifest['archive_url']


    elif args.cmd == 'list':
        list_builds()

'''

python publisher.py build --project=SuperKaijuVR.uproject --server-platform=Win64 --client-platform=Win64 --config=Debug
python publisher.py archive --image-folder=Saved\StagedBuilds\WindowsServer --archive-name=SuperKaijuVR-Win64-Debug-server-users.nonnib.93

python publisher.py publish --archive=G:\Super.nonnib.93.zip --repository=directivegames/SuperKaijuVR --ref=users/nonnib



'''