
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope -Latest

cd "$($name):"

$x = [string] (Get-Content "$($PSHOME)\types.ps1xml")

New-H5Attribute / vlen_str $x ustring

$y = $x.Substring(0,63000)

New-H5Attribute / fixed_str1 $y "ustring$($y.Length)"
New-H5Attribute / fixed_str2 $y "ustring$($y.Length)"
New-H5Attribute / fixed_str3 $y "ustring$($y.Length)"
New-H5Attribute / fixed_str4 $y "ustring$($y.Length)"
New-H5Attribute / fixed_str5 $y "ustring$($y.Length)"
New-H5Attribute / fixed_str6 $y "ustring$($y.Length)"
New-H5Attribute / fixed_str7 $y "ustring$($y.Length)"
New-H5Attribute / fixed_str8 $y "ustring$($y.Length)"

New-H5Attribute / fixed_str9 $x "ustring$($x.Length)"


New-H5Dataset fixed_str "ustring$($x.Length)" -Scalar

Set-H5datasetValue fixed_str $x

c:

Remove-H5Drive $name -Scope $scope
