
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
    New-H5Dataset $f $f 3,2
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

# null attributes

foreach ($f in $flavors)
{
    New-H5Attribute . "$f Null" -Nulll -Type $f
}

# scalar attributes

foreach ($f in $flavors)
{
    foreach ($i in 0..2) {
        foreach ($j in 0..1) {
            New-H5Attribute . "$f $i $j" -Scalar $value[$i, $j] $f
            $check = (Get-H5Attribute  . "$f $i $j").Value
            if ($value[$i, $j] -ne $check) {
                Write-Error "$($value[$i, $j]) != $($check)"
            }
        }
    }
}

# simple attributes

foreach ($f in $flavors)
{
    New-H5Attribute . $f -Simple 3,2 -Type $f -Value $value
    $check = (Get-H5Attribute  . $f).Value
    foreach ($i in 0..2) {
        foreach ($j in 0..1) {
            if ($value[$i, $j] -ne $check[$i, $j]) {
                Write-Error "$($value[$i, $j]) != $($check[$i, $j])"
            }
        }
    }
}

c:

Remove-H5Drive $name -Scope $scope 
