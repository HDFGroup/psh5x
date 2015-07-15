
Write-Host "$($MyInvocation.MyCommand.Path)" -BackgroundColor Yellow 

$ScriptDir = Split-Path $MyInvocation.MyCommand.Path
. $ScriptDir\Check-HDF5SnapinFunction.ps1
. $ScriptDir\Check-OpenHandlesFunction.ps1
. $ScriptDir\Print-MessagesFunctions.ps1

Check-HDF5Snapin

. $ScriptDir\Create-BNFSample.ps1

################################################################################
CreateHeading '0001' 'Get root of h5tmp'

$ret = $False 
$ret = (Get-Item h5tmp:\ -Detailed).Address -eq (Get-Item h5tmp: -Detailed).Address

ShowValues $True $ret; CreateFooter

################################################################################
CreateHeading '0002' 'Test wildcard'

$count = 0
Get-Item h5tmp:\* | % { $count++ }

ShowValues 7 $count; CreateFooter


Check-OpenHandles
