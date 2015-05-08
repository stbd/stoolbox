# To run this file, you might need change the default execution policy
# An example how to run:
# powershell -ExecutionPolicy ByPass -File install-freetype-windows.ps1

# Notes:
# MSBuild.exe needs to be on path, it can be found (sometimes at least) at c:/Program Files (x86)/MSBuild/12.0/bin/
# vcupgrade.exe needs to be on path, example location: c:/Program Files (x86)/Microsoft Visual Studio 12.0/Common7/Tools/vcupgrade.exe

$nproc=(Get-ChildItem Env:NUMBER_OF_PROCESSORS).Value
$path="$PSScriptRoot"
$url="http://download.savannah.gnu.org/releases/freetype/freetype-2.5.5.tar.bz2"

$version="2.5.5"
$file1="$path\freetype_$version.tar.bz2"
$file2="$path\freetype_$version.tar"
$folder="$path\freetype-$version"
$target="$path\freetype"

if (!(Test-Path "$folder")) {
    echo "Downloading and unpacking Freetype"

    Invoke-WebRequest "$url" -OutFile "$file1" -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox
    7z x "$file1"
    7z x "$file2"
}

# Use VC 2008 project as it can be converted to VC 2012
# The VC 2010 project is not compatible with VC 2012, and cannot be converted into one
# (apparently this is a bug)
vcupgrade.exe "$folder\builds\windows\vc2008\freetype.vcproj"
MSBuild.exe "$folder\builds\windows\vc2008\freetype.vcxproj" "/property:Configuration=Release"

mkdir -Force "$target"
mkdir -Force "$target\include"
mkdir -Force "$target\lib"
cp -Force "$path/freetype-$version/objs/win32/vc2008/freetype255.lib" "$target\lib"
cp -Force -Recurse "$path/freetype-$version/include" "$target\"

echo "Freetype installation done"
echo "Include: $target/include"
echo "Lib: $target/lib"
