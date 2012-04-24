
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope 

cd "$($name):"

$value = 'Meter per second'
New-H5Attribute "$($name):" Units $value 

$data = (Get-H5Attribute "$($name):" Units).Value

if ($value -ne $data) {
    Write-Error "'$value' != '$data'"
}

$value = 100,200
New-H5Attribute "$($name):" Speed -Simple 2 int $value 

$data = (Get-ItemProperty "$($name):" Speed).Value

for ($i = 0; $i -lt $value.Length; ++$i)
{
    if ($value[$i] -ne $data[$i]) {
        Write-Error "$i : $($value[$i]) != $($data[$i])"
    }
}


c:

Remove-H5Drive $name -Scope $scope 
