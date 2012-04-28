
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope -Core

cd "$($name):"

New-H5Dataset DS1 int 4,7 -Max -1,-1 -Chunk 3,3

$value = New-Object 'int[,]' 4,7

Set-H5DatasetValue DS1 $value

$value = Get-H5DatasetValue DS1

Format-H5Dataset DS1 6,7

Set-H5DatasetValue DS1 1 -Start 4,0 -Block 2,7

Format-H5Dataset DS1 6,10

Set-H5DatasetValue DS1 2 -Start 0,7 -Block 6,3

$value = Get-H5DatasetValue DS1

c:

Remove-H5Drive $name -Scope $scope 
