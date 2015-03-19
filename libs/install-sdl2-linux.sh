#!/bin/bash -eu

path=$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")
url=https://www.libsdl.org/release/SDL2-2.0.3.tar.gz
version=2.0.3
file="$path"/SDL2-"$version".tar.gz
folder="$path"/SDL2-"$version"
target="$path"/sdl2

if [ ! -d $folder ]; then
    rm -rf "$file"
    wget "$url" -O "$file"
    tar -xf "$file" -C "$path"
fi

mkdir -p "$folder"/build
cd "$folder"/build
cmake .. -DCMAKE_INSTALL_PREFIX="$target"
make install -j $(nproc)

echo "SDL installation done"
echo "Include path: $target/include"
echo "Library path: $target/lib"
