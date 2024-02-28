#!/bin/sh -e

cat > depot_client_main.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "367541"

  "FileMapping"
  {
    "LocalPath" "assets/*"
    "DepotPath" "assets/"
    "recursive" "1"
  }

  "FileMapping"
  {
    "LocalPath" "doc/*"
    "DepotPath" "doc/"
    "recursive" "1"
  }

  "FileMapping"
  {
    "LocalPath" "mods/*"
    "DepotPath" "mods/"
    "recursive" "1"
  }

  "FileMapping"
  {
    "LocalPath" "tiled/*"
    "DepotPath" "tiled/"
    "recursive" "1"
  }
}
EOF

cat > depot_client_win32.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "367542"

  "FileMapping"
  {
    "LocalPath" "win32/*"
    "DepotPath" "win32/"
    "recursive" "1"
  }
}
EOF

cat > depot_client_win64.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "367543"

  "FileMapping"
  {
    "LocalPath" "win64/*"
    "DepotPath" "win64/"
    "recursive" "1"
  }
}
EOF

cat > depot_client_macos.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "367544"

  "FileMapping"
  {
    "LocalPath" "osx/*"
    "DepotPath" "osx/"
    "recursive" "1"
  }
}
EOF

cat > depot_client_linux.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "367545"

  "FileMapping"
  {
    "LocalPath" "linux/*"
    "DepotPath" "linux/"
    "recursive" "1"
  }
}
EOF

cat > app_build_client.vdf << EOF
"appbuild"
{
  "appid" "367540"
  "desc" "$CI_BUILD_REF"
  "buildoutput" "steambuild_output"
  "contentroot" "client_distribution"
  "setlive" "staging"
  "preview" "0"
  "local" ""

  "depots"
  {
    "367541" "depot_client_main.vdf"
    "367542" "depot_client_win32.vdf"
    "367543" "depot_client_win64.vdf"
    "367544" "depot_client_macos.vdf"
    "367545" "depot_client_linux.vdf"
  }
}
EOF

$STEAM_CONTENTBUILDER_CMD +login "$STEAM_ACCOUNT" "$STEAM_PASSWORD" +run_app_build "`pwd`/app_build_client.vdf" +quit

cat > depot_server_main.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "532331"

  "FileMapping"
  {
    "LocalPath" "assets/*"
    "DepotPath" "assets/"
    "recursive" "1"
  }
}
EOF

cat > depot_server_win64.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "532332"

  "FileMapping"
  {
    "LocalPath" "win64/*"
    "DepotPath" "win64/"
    "recursive" "1"
  }
}
EOF

cat > depot_server_linux.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "532333"

  "FileMapping"
  {
    "LocalPath" "linux/*"
    "DepotPath" "linux/"
    "recursive" "1"
  }
}
EOF

cat > app_build_server.vdf << EOF
"appbuild"
{
  "appid" "532330"
  "desc" "$CI_BUILD_REF"
  "buildoutput" "steambuild_output"
  "contentroot" "server_distribution"
  "setlive" "staging"
  "preview" "0"
  "local" ""

  "depots"
  {
    "532331" "depot_server_main.vdf"
    "532332" "depot_server_win64.vdf"
    "532333" "depot_server_linux.vdf"
  }
}
EOF

$STEAM_CONTENTBUILDER_CMD +login "$STEAM_ACCOUNT" "$STEAM_PASSWORD" +run_app_build "`pwd`/app_build_server.vdf" +quit
