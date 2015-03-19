# To run this file, you might need change the default execution policy
# An example how to run:
# powershell  -ExecutionPolicy ByPass -File install-glm-windows.ps1

$path="$PSScriptRoot"
$url="http://downloads.sourceforge.net/project/ogl-math/glm-0.9.6.3/glm-0.9.6.3.7z?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fogl-math%2Ffiles%2Fglm-0.9.6.3%2F&ts=1426004603&use_mirror=garr"

$version="0.9.6.3"
$file="$path/glm-$version.7z"
$folder="$path/glm_$version"
$target="$path/glm"
$build_folder="$folder/glm/build"

if (!(Test-Path "$folder")) {
    echo "Downloading GLM"
    Invoke-WebRequest "$url" -OutFile "$file" -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox
    7z x "$file" -o"$folder"
}

rm  -Recurse -Force "$target" -ea SilentlyContinue
mkdir -Force "$build_folder"
cd "$build_folder"
cmake .. "-DCMAKE_INSTALL_PREFIX=$target"
cmake --build . --target install --config Release

echo "GLM installation done to $target"
