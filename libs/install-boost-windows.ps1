# To run this file, you might need change the default execution policy
# An example how to run:
# powershell -ExecutionPolicy ByPass -File install-boost-windows.ps1

$nproc=(Get-ChildItem Env:NUMBER_OF_PROCESSORS).Value
$path="$PSScriptRoot"
$url="http://downloads.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.7z?r=http%3A%2F%2Fwww.boost.org%2Fusers%2Fhistory%2Fversion_1_57_0.html&ts=1426093589&use_mirror=netcologne"

$version="1_57_0"
$file="$path\boost_$version.7z"
$folder="$path\boost_$version"
$target="$path\boost"

$cmd_bootstrap="bootstrap.bat"
$cmd_build="b2.exe variant=debug -j $nproc --prefix=$target install"

if (!(Test-Path "$folder")) {
    echo "Downloading and unpacking Boost"

    Invoke-WebRequest "$url" -OutFile "$file" -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox
    7z x "$file" -o"$path"
}

rm  -Recurse -Force "$target" -ea SilentlyContinue
cd "$folder"
bootstrap.bat
b2 "variant=debug,release" -j "$nproc" "--prefix=$target" install
