
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

$x = [string] (Get-Content "$($PSHOME)\types.ps1xml")

New-H5Attribute / vlen_str $x ustring

$y = $x.Substring(0,64000)

New-H5Attribute / fixed_str $y "ustring$($y.Length)"

New-H5Dataset fixed_str "ustring$($x.Length)" -Scalar

Set-H5datasetValue fixed_str $x

c:

Remove-H5Drive $name -Scope $scope 
