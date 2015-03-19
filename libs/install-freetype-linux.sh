#!/bin/bash -eu

path=$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")
url=http://download.savannah.gnu.org/releases/freetype/freetype-2.5.5.tar.gz
version=2.5.5
file="$path"/freetype-"${version}".tar.gz
folder="$path"/freetype-"${version}"
target_path="$path"/freetype

if [ ! -d $folder ]; then
    rm -rf "$file"
    wget "$url" -O "$file"
    tar -xf "$file" -C "$path"
fi

mkdir -p "$folder"/build
cd  "$folder"/build
cmake .. -DCMAKE_INSTALL_PREFIX="$target_path"
make -j $(nproc) install

echo "Freetype installation done"
echo "Include path: $target_path/include"
echo "Library path: $target_path/lib"
