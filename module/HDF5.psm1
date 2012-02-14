
$ScriptDir = Split-Path $MyInvocation.MyCommand.Path

Import-Module $ScriptDir\PSH5X.dll

. $ScriptDir\Get-H5Drive.ps1
. $ScriptDir\New-H5Attribute.ps1
. $ScriptDir\New-H5Dataset.ps1
. $ScriptDir\New-H5Drive.ps1
. $ScriptDir\New-H5File.ps1
. $ScriptDir\New-H5Group.ps1
. $ScriptDir\New-H5Hardlink.ps1
. $ScriptDir\Remove-H5Drive.ps1
