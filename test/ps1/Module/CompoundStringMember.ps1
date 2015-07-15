
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

$t = @"
{
  "Class":"Compound",
  "Size":"32",
  "Members":
  {
      "serial_no":[0, "H5T_STD_I64BE"],
      "location":[8, "string"],
      "temperature":[16, "double"],
      "pressure":[24, "double"]
  }
}
"@

Add-Type @"
using System;
public class cmpd 
{
    public int serial_no;
    public String location;
    public float temperature;
    public double pressure;
}
"@

New-H5Dataset dset1 $t 4 

$a = New-Object 'cmpd[]' 4
foreach ($i in 0..3)
{
    $a[$i] = New-Object cmpd
}

$a[0].serial_no = 1153
$a[0].location = 'Exterior (static)'
$a[0].temperature = 53.23
$a[0].pressure = 24.57
$a[1].serial_no = 1184
$a[1].location = 'Intake'
$a[1].temperature = 55.12
$a[1].pressure = 22.95
$a[2].serial_no = 1027
$a[2].location = 'Intake manifold'
$a[2].temperature = 103.55
$a[2].pressure = 31.23
$a[3].serial_no = 1313
$a[3].location = 'Exhaust manifold'
$a[3].temperature = 1252.89
$a[3].pressure = 84.11

Set-H5DatasetValue dset1 $a

$dummy = Get-H5DatasetValue dset1

$t = @"
{
  "Class":"Compound",
  "Size":"41",
  "Members":
  {
      "serial_no":[0, "H5T_STD_I64BE"],
      "location":[8, "cstring17"],
      "temperature":[25, "double"],
      "pressure":[33, "double"]
  }
}
"@

New-H5Dataset dset2 $t 4 

$a = New-Object 'cmpd[]' 4
foreach ($i in 0..3)
{
    $a[$i] = New-Object cmpd
}

$a[0].serial_no = 1153
$a[0].location = 'Exterior (static)'
$a[0].temperature = 53.23
$a[0].pressure = 24.57
$a[1].serial_no = 1184
$a[1].location = 'Intake'
$a[1].temperature = 55.12
$a[1].pressure = 22.95
$a[2].serial_no = 1027
$a[2].location = 'Intake manifold'
$a[2].temperature = 103.55
$a[2].pressure = 31.23
$a[3].serial_no = 1313
$a[3].location = 'Exhaust manifold'
$a[3].temperature = 1252.89
$a[3].pressure = 84.11

Set-H5DatasetValue dset2 $a

$dummy = Get-H5DatasetValue dset2

c:

Remove-H5Drive $name -Scope $scope
