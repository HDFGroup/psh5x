
$here = Split-Path $script:MyInvocation.MyCommand.Path

$compress = 'C:\Program Files\7-Zip\7z.exe'

$module = (Resolve-Path $here\module)
$config = (Resolve-Path $here\*.config)

$tempdir = [System.IO.Path]::GetTempPath()

$packdir = Join-Path $tempdir PSH5X

if (Test-Path $packdir) {
    del $packdir -Recurse -Force
}

mkdir $packdir,$packdir\module,$packdir\module\en-US,$packdir\Win32,$packdir\Win64

copy $config $packdir

copy $module\*.ps1 $packdir\module
copy $module\*.psm1 $packdir\module
copy $module\*.psd1 $packdir\module
copy $module\*.ps1xml $packdir\module

copy $module\en-US\*.txt $packdir\module\en-US

copy $here\Win32\Release\PSH5X.dll $packdir\Win32
copy $here\Win64\Release\PSH5X.dll $packdir\Win64

copy "${Env:ProgramFiles(x86)}\HDF Group\HDF5" $packdir\Win32 -Recurse
copy "$Env:ProgramFiles\HDF Group\HDF5" $packdir\Win64 -Recurse

copy $here\Install-PSH5X.ps1 $packdir

$args = ('a', "$tempdir\PSH5X.zip", $packdir)

& "$compress" $args

move $tempdir\PSH5X.zip . -Force

del $packdir -Recurse -Force
