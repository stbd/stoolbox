# To run this file, you might need change the default execution policy
# An example how to run:
# powershell  -ExecutionPolicy ByPass -File install-sdl2-windows.ps1

$path="$PSScriptRoot"
$version="2.0.3"
$url="https://www.libsdl.org/release/SDL2-$version.tar.gz"
$file_tar="$path/SDL2-$version.tar"
$file="$file_tar.gz"
$folder="$path/SDL2-$version"
$target="$path/sdl2"
$build_folder="$folder/SDL2-$version/build"

if (!(Test-Path "$folder")) {
    echo "Downloading SDL"
    Invoke-WebRequest "$url" -OutFile "$file" -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox
    7z e "$file" -o"$file_tar"
    7z x "$file_tar" -o"$folder"
}

rm  -Recurse -Force "$target" -ea SilentlyContinue
mkdir -Force "$build_folder"
cd "$build_folder"
cmake .. "-DCMAKE_INSTALL_PREFIX=$target"
cmake --build . --target install --config Release

# For some reason, when both boost and SDL2 are statistically linked in SToolbox,
# boost Log throws an exception related to Gregorian year, thus only use dynamically
# linked SDL

# Instal target apparently depends on all other targets, and all files are installed to target path
# This is a problem because project has targets for both statically and dynamicly linked 
# projects, and both get installed by default

# Workaround for this is to first run install target so that all headers get installed
# Then manually remove installed libraries, rebuild only dynamic version of library
# and copy that files manually to target path

rm -Force -Recurse "$build_folder/Release/*"
cmake --build . --target SDL2 --config Release
cmake --build . --target SDL2main --config Release
cp "$build_folder/Release/*.lib" "$target/lib"

echo "SDL installation done to path $target"
echo "Remember to somehow add $target/bin to path"
