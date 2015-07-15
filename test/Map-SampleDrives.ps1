Write-Host "$($MyInvocation.MyCommand.Path)" -BackgroundColor Yellow 

$ScriptDir = Split-Path $MyInvocation.MyCommand.Path
. $ScriptDir\Check-HDF5SnapinFunction.ps1
. $ScriptDir\Print-MessagesFunctions.ps1

Check-HDF5Snapin

$files = Get-ChildItem "$ScriptDir\h5\h5ex_t*.h5"

foreach ($f in $files)
{
  $drive = $f.Name.Split('.')[0].Substring(7);
  $ret = New-PSDrive $drive -PSProvider HDF5 -Root "$($drive):\" -Path $f.FullName -Scope 1
}
