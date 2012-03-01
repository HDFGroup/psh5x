
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-PSDrive $name HDF5 "$($name):\" -Path $path -Mode RW -Force -Scope $scope 

cd "$($name):"

New-ItemProperty . attr1 -ElementType string17

Set-ItemProperty . attr1 'string attribute'

New-Item dset1 -ItemType Dataset -ElementType H5T_STD_I32BE -Dimensions 10,10

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

New-Item dset2 -ItemType Dataset -ElementType $t -Dimensions 5 

New-Item group1 -ItemType Group

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

New-Item type1 -ItemType Datatype -Definition $t

cd group1

New-Item dset3 -ItemType Dataset -ElementType /type1 -Dimensions 5 

cd ..

$t = '{"Class": "Vlen", "Base": "H5T_STD_I32LE"}'

New-Item dset3 -ItemType Dataset -ElementType $t -Dimensions 4 

$value = @(@(0), @(10,11), @(20,21,22), @(30,31,32,33))

Set-Content dset3 $value

New-Item group2 -ItemType Hardlink -Value /group1

New-Item slink1 -ItemType Softlink -Value somevalue 

c:

Remove-PSDrive $name -Scope $scope 
