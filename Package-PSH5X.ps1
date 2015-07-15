
$here = Split-Path $script:MyInvocation.MyCommand.Path

$compress = 'C:\Program Files\7-Zip\7z.exe'

$module = (Resolve-Path $here\module)
$config = (Resolve-Path $here\*.config)

$tempdir = [System.IO.Path]::GetTempPath()

$packdir = Join-Path $tempdir PSH5X

if (Test-Path $packdir) {
    del $packdir -Recurse -Force
}

mkdir $packdir,$packdir\examples,$packdir\module,$packdir\module\en-US,$packdir\Win32,$packdir\Win64

# module files

copy $module\*.ps1 $packdir\module
copy $module\*.psm1 $packdir\module
copy $module\*.psd1 $packdir\module
copy $module\*.ps1xml $packdir\module
copy $module\en-US\*.txt $packdir\module\en-US

# PSH5X binaries

copy $here\Win32\Release\PSH5X.dll $packdir\Win32
copy $here\Win64\Release\PSH5X.dll $packdir\Win64

# MSVC dlls

copy "${Env:ProgramFiles(x86)}\HDF Group\HDF5\1.8.9\static\bin\msvc*.dll" $packdir\Win32
copy "$Env:ProgramFiles\HDF Group\HDF5\1.8.9\static\bin\msvc*.dll" $packdir\Win64

# Misc

copy $config $packdir
copy $here\Install-PSH5X.ps1 $packdir
copy $here\COPYING $packdir
#copy $here\README $packdir
copy $here\psh5x_logo.gif $packdir
copy $here\doc\INSTALL $packdir
copy $here\doc\RELEASE.txt $packdir

copy $here\doc\demo.txt $packdir\examples
copy $here\doc\Start-Demo.ps1 $packdir\examples
copy $here\doc\sample.ps1 $packdir\examples
copy $here\doc\hdf_logo.jpg $packdir\examples
copy $here\doc\PSH5X.png $packdir\examples

$args = ('a', "$tempdir\PSH5X.zip", $packdir)

& "$compress" $args

move $tempdir\PSH5X.zip . -Force

del $packdir -Recurse -Force
