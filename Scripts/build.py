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

def get_project_file(project_filename):
    if project_filename:
        if not project_filename.lower().endswith(".uproject"):
            print "Project file must end with .uproject"
            sys.exit(1)

        if not os.path.exists(project_filename):
            print "Project file '%s' not found. Please note that this must be an absolute path" % project_filename
            sys.exit(1)
        project_name = os.path.splitext(os.path.basename(project_filename))[0]
    else:
        # assume the project file is one level above this script
        path = os.path.abspath(os.path.join(get_script_path(), "..\\")).replace('\\', '/')
        project_name = path.split('/')[-1]
        project_filename = os.path.join(path, project_name) + ".uproject"
        project_filename = os.path.normpath(project_filename)
        if not os.path.exists(project_filename):
            print 'No .uproject file found in parent folder. Please specify the path to your project file with the --project parameter'
            sys.exit(1)
    return project_name, project_filename

# Build platform specifics
if sys.platform == 'win32':
    RUNUAT = 'RunUAT.bat'
elif sys.platform == 'darwin':
    RUNUAT = 'RunUAT.command'
elif sys.platform == 'linux':
    RUNUAT = 'RunUAT.sh'
else:
    raise RuntimeError("Build platform %s not supported.", sys.platform)

def get_staging_directory(project_file, config_name):
    project_root, _ = os.path.split(project_file)
    project_root = os.path.abspath(project_root)
    return os.path.join(project_root, 'Saved', 'StagedBuilds', config_name)


def get_ue4_root(project_name):
    """Figure out which UE4 engine source is associated with 'project_name'."""
    with open(project_name) as f:
        project = json.load(f)

    if not project['EngineAssociation'].startswith("{"):
        print "\nProject is using stock engine. With this tool you can only build from Engine source.\nPlease switch engine versions in %s" % project_name
        sys.exit(1)
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
        '-project="' + os.path.abspath(project_file).replace('\\', '\\\\') + '"',
        '-noP4',
        '-clientconfig=' + config_name,
        '-serverconfig=' + config_name,
        '-nocompileeditor',
        '-ue4exe=UE4Editor-Cmd.exe',
        '-cook',
        '-build',
        '-stage',
        '-pak',
        '-iterativecooking ',
        '-compressed',
        '-archive',
        '-stagingdirectory="%s"' % get_staging_directory(project_file, config_name)
    ]
    if map_name:
        cmd.append('-map='+map_name)
    else:
        cmd.append('-allmaps')

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


if __name__ == "__main__":
    start_time = time.time()
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--server-platform', default='Win64', help="Platform target for server")
    parser.add_argument('-c', '--client-platform', required=False, help="Platform target for client if that should be built as well")
    parser.add_argument('--cfg', default='Development', help="Build configuration to use (default is Development)")
    parser.add_argument('-p', '--project', required=False, help=".uproject file to use. If not specified assumes it is in the parent folder of this script")
    parser.add_argument('-m', '--map-name', required=False, help="Comma separated list of maps to include in the build (default is all maps)")
    args = parser.parse_args()

    tp_archives = {}  # Key is target platform, value is archive folder, zip file name.
    build_manifests = []

    project_name, project_file = get_project_file(args.project)

    server_platform = args.server_platform
    config_name = args.cfg

    print "Building project %s..." % project_name
    client_platform = None
    if args.client_platform:
        client_platform = args.client_platform

    target_platforms = build_project(
        project_file=project_file,
        config_name=config_name,
        server_platform=server_platform,
        client_platform=client_platform,
        map_name=args.map_name
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
