
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-PSDrive $name HDF5 "$($name):\" -Path $path -Mode RW -Force -Scope $scope -Core

cd "$($name):"

# test code goes here

c:

Remove-PSDrive $name -Scope $scope 
