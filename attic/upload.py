#!/usr/bin/env python2.7

import os
import sys
import time
import shutil
import ConfigParser

currentStorageDir = 'giraffe_storage'
maxDirectoryAge = 86400
privateTargetDir = os.path.expanduser('/mnt/chuckleshare/buildbot/')
steamContentBuilderDir = os.path.expanduser('~/steam_uploader')

cfg = ConfigParser.SafeConfigParser()
cfg.read(os.path.expanduser("~/passwords.cfg"))
steamAccount = cfg.get("steam", "account")
steamPassword = cfg.get("steam", "password")

def copyAndReplace(source, dest, replacements):
  sourcefile = open(source)
  destfile = open(dest, 'w')

  for line in sourcefile:
    for src, target in replacements.items():
      line = line.replace(src, target)
    destfile.write(line)
  sourcefile.close()
  destfile.close()

def touch(file):
  open(file, 'w').write("")

def copyTreeExisting(src, dst, symlinks=False, ignore=None):
  if not os.path.exists(dst):
    os.makedirs(dst)
  for item in os.listdir(src):
    s = os.path.join(src, item)
    d = os.path.join(dst, item)
    if os.path.isdir(s):
      copyTreeExisting(s, d, symlinks, ignore)
    else:
      shutil.copy2(s, d)

def recursiveChmod(targetDir, level):
  for root, dirs, files in os.walk(targetDir):
    for entry in dirs:
      os.chmod(os.path.join(root, entry), level | 0o111)
    for entry in files:
      os.chmod(os.path.join(root, entry), level)
    os.chmod(root, level | 0o111)

def basename(dir):
  bn = os.path.basename(dir)
  if bn == '':
    return os.path.basename(os.path.dirname(dir))
  else:
    return bn

def rmtree(dir):
  if os.path.isdir(dir):
    shutil.rmtree(dir)
  elif os.path.exists(dir):
    sys.exit("Directory %s expected, file found instead, aborting" % dir)

def uploadNewPrivateRevision(revisionDir, revision):
  print("Uploading new private revision...")
  targetDir = os.path.join(privateTargetDir, revision)

  rmtree(targetDir)
  os.mkdir(targetDir)

  pcdir = os.path.join(targetDir, "pc")
  os.mkdir(pcdir)

  shutil.copytree(os.path.join(revisionDir, "data/assets"), os.path.join(pcdir, "assets"))
  shutil.copytree(os.path.join(revisionDir, "data/tiled"), os.path.join(pcdir, "tiled"))
  shutil.copytree(os.path.join(revisionDir, "linux32_binaries"), os.path.join(pcdir, "linux32"))
  shutil.copytree(os.path.join(revisionDir, "linux64_binaries"), os.path.join(pcdir, "linux64"))
  shutil.copytree(os.path.join(revisionDir, "osx_binaries"), os.path.join(pcdir, "osx"))
  shutil.copytree(os.path.join(revisionDir, "win32_binaries"), os.path.join(pcdir, "win32"))
  shutil.copytree(os.path.join(revisionDir, "win64_binaries"), os.path.join(pcdir, "win64"))

  xboxdir = os.path.join(targetDir, "xboxone")
  os.mkdir(xboxdir)
  shutil.copytree(os.path.join(revisionDir, "data/assets"), os.path.join(xboxdir, "assets"))
  copyTreeExisting(os.path.join(revisionDir, "xboxone_binaries"), xboxdir)

  recursiveChmod(targetDir, 0o644)

  print("Finished uploading private revision!")

def uploadNewSteamRevision(revisionDir, isStable, setLive, revision):
  print("Uploading new steam revision...")

  if setLive != '':
    print("This will set the content live to the steam branch '%s'" % steamLiveBranch)

  print("Removing old steam content structure...")
  steamContentDir = os.path.join(steamContentBuilderDir, "content")
  rmtree(steamContentDir)

  print("Building new steam content structure...")
  os.mkdir(steamContentDir)
  shutil.copytree(os.path.join(revisionDir, "data/assets"), os.path.join(steamContentDir, "assets"))
  shutil.copytree(os.path.join(revisionDir, "data/tiled"), os.path.join(steamContentDir, "tiled"))
  shutil.copytree(os.path.join(revisionDir, "linux32_binaries"), os.path.join(steamContentDir, "linux32"))
  shutil.copytree(os.path.join(revisionDir, "linux64_binaries"), os.path.join(steamContentDir, "linux64"))
  shutil.copytree(os.path.join(revisionDir, "osx_binaries"), os.path.join(steamContentDir, "osx"))
  shutil.copytree(os.path.join(revisionDir, "win32_binaries"), os.path.join(steamContentDir, "win32"))
  shutil.copytree(os.path.join(revisionDir, "win64_binaries"), os.path.join(steamContentDir, "win64"))

  os.mkdir(os.path.join(steamContentDir, currentStorageDir))
  os.mkdir(os.path.join(steamContentDir, currentStorageDir, "mods"))
  touch(os.path.join(steamContentDir, currentStorageDir, "mods", "mods_go_here"))

  if isStable:
    scriptsDir = os.path.join(steamContentBuilderDir, "scripts")
    copyAndReplace(os.path.join(scriptsDir, "app_build_stable.vdf.template"), os.path.join(scriptsDir, "app_build_stable.vdf"),
      {'{REVISION}' : revision, '{SETLIVE}' : setLive})

    print("Uploading content to Steam STABLE build, Steam uploader output follows...")

    os.chdir(os.path.join(steamContentBuilderDir, "builder"))
    if os.system("./steamcmd.sh +login %s '%s' +run_app_build ../scripts/app_build_stable.vdf +quit 2>&1" % (steamAccount, steamPassword)) != 0:
      sys.exit("Something went wrong with the steam upload, aborting!")

  else:
    scriptsDir = os.path.join(steamContentBuilderDir, "scripts")
    copyAndReplace(os.path.join(scriptsDir, "app_build_unstable.vdf.template"), os.path.join(scriptsDir, "app_build_unstable.vdf"),
      {'{REVISION}' : revision, '{SETLIVE}' : setLive})

    print("Uploading content to Steam UNSTABLE build, Steam uploader output follows...")

    os.chdir(os.path.join(steamContentBuilderDir, "builder"))
    if os.system("./steamcmd.sh +login %s '%s' +run_app_build ../scripts/app_build_unstable.vdf +quit 2>&1" % (steamAccount, steamPassword)) != 0:
      sys.exit("Something went wrong with the steam upload, aborting!")

  print("Finished uploading build to steam!")

directory = sys.argv[0]
revision = sys.argv[1]
steamTarget = sys.argv[2] if len(sys.argv) >= 2 else "none"
steamLiveBranch = sys.argv[3] if len(sys.argv) >= 3 else ""

# Change stdout to be unbuffered for buildbot logging
sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

print("Uploading new starbound revision %s..." % revision)
outputDirs = set(os.listdir(revisionDir))

requiredDirs = set(['linux32_binaries', 'linux64_binaries', 'osx_binaries', 'win32_binaries', 'win64_binaries', 'xboxone_binaries', 'data'])
if not outputDirs.issuperset(requiredDirs):
  sys.exit("Refusing to upload %s due to missing directories: %s" % (revision, requiredDirs.difference(outputDirs)))

uploadNewPrivateRevision(revisionDir, basename(revisionDir))

if steamTarget == 'stable':
  uploadNewSteamRevision(revisionDir, True, setLive, basename(revisionDir))
elif steamTarget == 'unstable':
  uploadNewSteamRevision(revisionDir, False, setLive, basename(revisionDir))
