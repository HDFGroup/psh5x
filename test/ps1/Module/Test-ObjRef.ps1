
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

New-H5Group G1

New-H5Dataset DS2 int -Scalar

New-H5Dataset DS1 ObjRef -Dim 2

Set-H5DatasetValue DS1 '/G1','/DS2'

c:

Remove-H5Drive $name -Scope $scope 
