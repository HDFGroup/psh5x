
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-PSDrive $name HDF5 "$($name):\" -Path $path -Mode RW -Force -Scope $scope

cd "$($name):"

$value = New-Object 'string[,]' 3,2

$value[0,0] = 'DOMI'; $value[0,1] = 'MINA'
$value[1,0] = 'NVS' ; $value[1,1] = 'TIO'
$value[2,0] = 'ILLV'; $value[2,1] = 'MEA'

$flavors = @('string','string4','cstring5','fstring4','ucstring5','ufstring4')

foreach ($f in $flavors)
{
    $dset = New-Item $f -ItemType Dataset -ElementType $f -Dimensions 3,2
    Set-Content $f $value

    $a = Get-Content $f -ReadCount 0
    foreach ($i in 0..2) {
        foreach ($j in 0..1) {
            if ($value[$i, $j] -ne $a[$i, $j]) {
                Write-Error "$($value[$i, $j]) != $($a[$i, $j])"
            }
        }
    }

    $a = Get-Content $f

    $a = Get-Content $f -ReadCount 3
}

c:

Remove-PSDrive $name -Scope $scope 
