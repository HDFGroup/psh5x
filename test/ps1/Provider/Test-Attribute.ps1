
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-PSDrive $name HDF5 "$($name):\" -Path $path -Mode RW -Force -Scope $scope

cd "$($name):"

$value = 'Meter per second'
New-ItemProperty "$($name):" Units -ElementType string -Value $value 

$data = (Get-ItemProperty "$($name):" Units).Value

if ($value -ne $data) {
    Write-Error "'$value' != '$data'"
}

$value = 100,200
New-ItemProperty "$($name):" Speed -ElementType int -Simple 2 -Value $value 

$data = (Get-ItemProperty "$($name):" Speed).Value

for ($i = 0; $i -lt $value.Length; ++$i)
{
    if ($value[$i] -ne $data[$i]) {
        Write-Error "$i : $($value[$i]) != $($data[$i])"
    }
}

c:

Remove-PSDrive $name -Scope $scope 
