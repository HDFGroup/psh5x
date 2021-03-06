
$ScriptDir = Split-Path $MyInvocation.MyCommand.Path

Import-Module $ScriptDir\PSH5X.dll
Export-ModuleMember -Cmdlet * 

. $ScriptDir\Aliases.ps1
. $ScriptDir\Add-H5Packets.ps1
. $ScriptDir\Copy-H5Item.ps1
. $ScriptDir\Format-H5Dataset.ps1
. $ScriptDir\Get-H5Drive.ps1
. $ScriptDir\Get-H5DatasetValue.ps1
. $ScriptDir\New-H5Attribute.ps1
. $ScriptDir\New-H5Dataset.ps1
. $ScriptDir\New-H5Drive.ps1
. $ScriptDir\New-H5Extlink.ps1
. $ScriptDir\New-H5File.ps1
. $ScriptDir\New-H5Group.ps1
. $ScriptDir\New-H5Hardlink.ps1
. $ScriptDir\New-H5Image.ps1
. $ScriptDir\New-H5LinkedDatatype.ps1
. $ScriptDir\New-H5PacketTable.ps1
. $ScriptDir\New-H5NullDataset.ps1
. $ScriptDir\New-H5ScalarDataset.ps1
. $ScriptDir\New-H5Softlink.ps1
. $ScriptDir\Remove-H5Drive.ps1
. $ScriptDir\Set-H5DatasetValue.ps1
. $ScriptDir\Show-H5Drive.ps1
. $ScriptDir\Test-H5Attribute.ps1

Export-ModuleMember -Function * -Alias *
