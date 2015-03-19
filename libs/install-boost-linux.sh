#!/bin/bash -eu

path=$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")
url=http://sourceforge.net/projects/boost/files/boost/1.57.0/boost_1_57_0.tar.gz/download
version=1_57_0
file="$path"/boost_"${version}".tar.gz
folder="$path"/boost_"${version}"
target="$path"/boost

if [ ! -d $folder ]; then
    rm -rf "$file"
    wget "$url" -O "$file"
    tar -xf "$file" -C "$path"
fi

cd "$folder"
"$PWD"/bootstrap.sh --prefix=$target
"$PWD"/b2 variant=release -j $(nproc) install

echo "Boost installation done"
echo "Include path: $boost_target/include"
echo "Library path: $boost_target/lib"
