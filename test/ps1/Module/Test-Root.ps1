
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

$root = Get-H5Item . -Detailed

if ($root.ItemType -ne 'Group') {
    Write-Error "`$root.ItemType: $($root.ItemType) != 'Group'"
}

if ($root.ReferenceCount -ne 1) {
    Write-Error "`$root.ReferenceCount: $($root.ReferenceCount) != 1"
}

if ($root.ObjectID -ne 33554432) {
    Write-Error "`$root.ObjectID: $($root.ObjectID) != 33554432"
}

$root = Get-H5Item "$($name):" -Detailed

if ($root.ItemType -ne 'Group') {
    Write-Error "`$root.ItemType: $($root.ItemType) != 'Group'"
}

if ($root.ReferenceCount -ne 1) {
    Write-Error "`$root.ReferenceCount: $($root.ReferenceCount) != 1"
}

if ($root.ObjectID -ne 33554432) {
    Write-Error "`$root.ObjectID: $($root.ObjectID) != 33554432"
}

$root = Get-H5Item "$($name):\" -Detailed

if ($root.ItemType -ne 'Group') {
    Write-Error "`$root.ItemType: $($root.ItemType) != 'Group'"
}

if ($root.ReferenceCount -ne 1) {
    Write-Error "`$root.ReferenceCount: $($root.ReferenceCount) != 1"
}

if ($root.ObjectID -ne 33554432) {
    Write-Error "`$root.ObjectID: $($root.ObjectID) != 33554432"
}

$root = Get-H5Item "$($name):/" -Detailed

if ($root.ItemType -ne 'Group') {
    Write-Error "`$root.ItemType: $($root.ItemType) != 'Group'"
}

if ($root.ReferenceCount -ne 1) {
    Write-Error "`$root.ReferenceCount: $($root.ReferenceCount) != 1"
}

if ($root.ObjectID -ne 33554432) {
    Write-Error "`$root.ObjectID: $($root.ObjectID) != 33554432"
}

c:

Remove-H5Drive $name -Scope $scope 
