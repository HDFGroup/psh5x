
$count = 0
Get-PSSnapin -Registered | ?  {$_.Name -eq 'HDF5'} | % {$count++}
if ($count -ne 1)
{
    Throw 'The HDF5 plugin has not been registered on this machine!'
}

$count = 0
Get-PSSnapin | ? {$_.Name -eq 'HDF5'} | % {$count++}
if ($count -ne 1)
{
    Write-Host 'HDF5 snap-in not found. Loading...'; Add-PSSnapin HDF5
}

$count = 0
Get-PSSnapin | ? {$_.Name -eq 'HDF5'} | % {$count++}
if ($count -ne 1)
{
    Throw 'Failed to load HDF5 snap-in!'
}

$ScriptDir = Split-Path $MyInvocation.MyCommand.Path

. $ScriptDir\New-H5Drive.ps1
. $ScriptDir\New-H5File.ps1
