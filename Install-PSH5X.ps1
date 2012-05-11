
$here = Split-Path $script:MyInvocation.MyCommand.Path

if((Get-WmiObject -Class Win32_OperatingSystem).OSArchitecture -eq '64-bit') {
    copy $here\Win64\PSH5X.dll $here\module -Force
}
else {
    copy $here\Win32\PSH5X.dll $here\module -Force
}

$VistaPath = "$Env:USERPROFILE\Documents\WindowsPowerShell\Modules\HDF5"
$XPPath =  "$Env:USERPROFILE\My Documents\WindowsPowerShell\Modules\HDF5" 
$dest = $Null

if ([int]((Get-WmiObject -Class Win32_OperatingSystem).Version).Substring(0,1) -ge 6) { 
    if(-not(Test-Path -Path $VistaPath)) {
        New-Item -Path $VistaPath -ItemType Directory | Out-Null
    }
    $dest = $VistaPath
}
else {  
    if(-not(Test-Path -Path $XPPath)) {
        New-Item -Path $XPPath -ItemType Directory | Out-Null
    }
    $dest = $XPPath
}

if ($dest -ne $Null) {
    copy $here\module\* $dest -Force
    copy $here\COPYING $dest -Force
}

$check = (Get-Module -ListAvailable | ?{$_.Name -eq 'HDF5'})

if ($check -ne $Null) {
    'You are ready to roll. Enjoy HDF5!'
}
else {
    'Arghhh!!! Something went wrong. Contact The HDF Group to get help!'
}
