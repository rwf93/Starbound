#!/bin/sh -e

cat > depot_client_main.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "211821"

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
  "DepotID" "211822"

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
  "DepotID" "211823"

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
  "DepotID" "211824"

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
  "DepotID" "211825"

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
  "appid" "211820"
  "desc" "$CI_BUILD_REF"
  "buildoutput" "steambuild_output"
  "contentroot" "client_distribution"
  "setlive" "staging"
  "preview" "0"
  "local" ""

  "depots"
  {
    "211821" "depot_client_main.vdf"
    "211822" "depot_client_win32.vdf"
    "211823" "depot_client_win64.vdf"
    "211824" "depot_client_macos.vdf"
    "211825" "depot_client_linux.vdf"
  }
}
EOF

$STEAM_CONTENTBUILDER_CMD +login "$STEAM_ACCOUNT" "$STEAM_PASSWORD" +run_app_build "`pwd`/app_build_client.vdf" +quit

cat > depot_server_main.vdf << EOF
"DepotBuildConfig"
{
  "DepotID" "533831"

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
  "DepotID" "533832"

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
  "DepotID" "533833"

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
  "appid" "533830"
  "desc" "$CI_BUILD_REF"
  "buildoutput" "steambuild_output"
  "contentroot" "server_distribution"
  "setlive" "staging"
  "preview" "0"
  "local" ""

  "depots"
  {
    "533831" "depot_server_main.vdf"
    "533832" "depot_server_win64.vdf"
    "533833" "depot_server_linux.vdf"
  }
}
EOF

$STEAM_CONTENTBUILDER_CMD +login "$STEAM_ACCOUNT" "$STEAM_PASSWORD" +run_app_build "`pwd`/app_build_server.vdf" +quit
