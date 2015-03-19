#!/bin/bash -eu

path=$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")
glm_version=0.9.6.1
glm_file="$path"/glm-"${glm_version}".zip
glm_folder="$path"/glm_"${glm_version}"
glm_target="$path"/glm

if [ ! -d $glm_folder ]; then
    rm -rf "$glm_file"
    wget https://sourceforge.net/projects/ogl-math/files/glm-0.9.6.1/glm-0.9.6.1.zip/download -O "$glm_file"
    unzip "$glm_file" -d "$glm_folder"
fi

mkdir -p "$glm_folder"/glm/build
cd "$glm_folder"/glm/build
cmake .. -DCMAKE_INSTALL_PREFIX="$glm_target"
make install

echo "GLM installation done, add $glm_target/include to include paths"
