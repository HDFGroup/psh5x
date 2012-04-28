
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

$value = New-Object 'string[,]' 3,2

$value[0,0] = 'DOMI'; $value[0,1] = 'MINA'
$value[1,0] = 'NVS' ; $value[1,1] = 'TIO'
$value[2,0] = 'ILLV'; $value[2,1] = 'MEA'

$flavors = @('string','string4','cstring5','fstring4','ucstring5','ufstring4')

foreach ($f in $flavors)
{
    $dset = New-H5Dataset $f $f 3,2
    Set-H5DatasetValue $f $value

    $a = Get-H5DatasetValue $f
    foreach ($i in 0..2) {
        foreach ($j in 0..1) {
            if ($value[$i, $j] -ne $a[$i, $j]) {
                Write-Error "$($value[$i, $j]) != $($a[$i, $j])"
            }
        }
    }
}

c:

Remove-H5Drive $name -Scope $scope 
