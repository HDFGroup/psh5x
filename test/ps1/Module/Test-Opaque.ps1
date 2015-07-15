
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

[byte[]] $x = Get-Content -Encoding byte -Path "$($env:windir)\System32\notepad.exe" -ReadCount 0

New-H5Dataset notepad.exe "opaque$($x.Count)['notepad.exe']" -Scalar

Set-H5DatasetValue notepad.exe (,$x)

c:

Remove-H5Drive $name -Scope $scope 
