
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

$dummy = New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

$dummy = New-H5Attribute . attr1 'string attribute' string17

$dummy = New-H5Dataset dset1 H5T_STD_I32BE 10,10

$value = New-H5Array int 100

foreach ($i in 0..99) {
    $value[$i] = $i%10
}

Set-Content dset1 $value

$t = @"
{
    "Class": "Compound",
    "Size": 16,
    "Members":
    {
        "a": [0, "H5T_STD_I32BE"],
        "b": [4, "H5T_IEEE_F32BE"],
        "c": [8, "H5T_IEEE_F64BE"]
    }
}
"@

$dummy = New-H5Dataset dset2 $t 5

$dummy = New-H5Group group1

$t = @"
{
    "Class": "Compound",
    "Size": 136,
    "Members":
    {
        "a": [0, {"Class": "Array", "Base": "H5T_STD_I32BE", "Dims": [4]}],
        "b": [16,{"Class": "Array", "Base": "H5T_IEEE_F32BE", "Dims":[5,6]}]
    }
}
"@

$dummy = New-H5LinkedDatatype type1 $t

cd group1

$dummy = New-H5Dataset dset3 /type1 5

$t = '{"Class": "Vlen", "Base": "H5T_STD_I32LE"}'

cd ..

$dummy = New-H5Dataset dset3 $t 4

$value = @(@(0), @(10,11), @(20,21,22), @(30,31,32,33))

Set-Content dset3 $value

$dummy = New-H5Hardlink group2 /group1

$dummy = New-H5Softlink slink1 somevalue

c:

Remove-H5Drive $name -Scope $scope
