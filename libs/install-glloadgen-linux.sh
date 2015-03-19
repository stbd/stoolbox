#!/bin/bash -eu

path=$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")
url=https://bitbucket.org/alfonse/glloadgen/downloads/glLoadGen_2_0_2.7z
version=2_0_2
file="$path"/glLoadgen_"${version}".7z
folder="$path"/glLoadGen_"${version}"
target="$path"/gl

if [ ! -d $folder ]; then
    rm -rf "$file"
    wget "$url" -O "$file"
    7zr x "$file" -o"$path"
fi

cd "$folder"
mkdir -p "$target"
lua LoadGen.lua -spec=gl -version=3.2 "$target"/stb

echo "Gl installation done"
echo "Files generated to $target"
