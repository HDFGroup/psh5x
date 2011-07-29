
Write-Host "$($MyInvocation.MyCommand.Path)" -BackgroundColor Yellow 

$ScriptDir = Split-Path $MyInvocation.MyCommand.Path
. $ScriptDir\Check-HDF5SnapinFunction.ps1
. $ScriptDir\Print-MessagesFunctions.ps1

Check-HDF5Snapin

################################################################################
CreateHeading '0001' 'h5tmp exists'

$count = 0
Get-PSDrive | ? { $_.Name -eq 'h5tmp' } | % {$count++}

ShowValues 1 $count; CreateFooter

################################################################################
CreateHeading '0002' '$Env:PSH5XTmpFile defined'

$count = 0
Get-ChildItem env: | ? { $_.Name -eq 'PSH5XTmpFile' } | % {$count++}

ShowValues 1 $count; CreateFooter

################################################################################
CreateHeading '0003' 'Map drive RO'

$a = New-PSDrive -Name h5_1 -PSProvider HDF5 -Path $ScriptDir\sample.h5 -Root h5_1:\

$count = 0
Get-PSDrive | ? { $_.Name -eq 'h5_1' } | % {$count++}

ShowValues 1 $count; CreateFooter

################################################################################
CreateHeading '0004' 'Remove drive'

Remove-PSDrive h5_1

$count = 0
Get-PSDrive | ? { $_.Name -eq 'h5_1' } | % {$count++}

ShowValues 0 $count; CreateFooter

################################################################################
CreateHeading '0005' 'Map drive RW'

$a = New-PSDrive -Name h5_1 -PSProvider HDF5 -Path $ScriptDir\sample.h5 -Root h5_1:\ -Mode RW

$count = 0
Get-PSDrive | ? { $_.Name -eq 'h5_1' } | % {$count++}

ShowValues 1 $count; CreateFooter

################################################################################
CreateHeading '0006' 'Remove drive'

Remove-PSDrive h5_1

$count = 0
Get-PSDrive | ? { $_.Name -eq 'h5_1' } | % {$count++}

ShowValues 0 $count; CreateFooter

