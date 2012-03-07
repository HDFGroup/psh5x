
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

$dset = New-H5Dataset dset1 H5T_STD_I32BE 10,10

$value = New-Object 'int[,]' 10,10

foreach ($i in 0..9) {
    foreach ($j in 0..9) {
        $value[$i,$j] = $j
    }
}

Set-H5DatasetValue dset1 $value

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

Add-Type @"
public class cmpd_a_b_c
{
    public int a;
    public float b;
    public double c;
}
"@

$dset = New-H5Dataset dset2 $t 5

$value = New-Object 'cmpd_a_b_c[]' 5

foreach ($i in 1..5)
{
    $value[$i-1] = New-Object cmpd_a_b_c

    $value[$i-1].a = $i
    $value[$i-1].b = $i/10
    $value[$i-1].c = $i/100
}

Set-H5DatasetValue dset2 $value

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

Add-Type @"
public class cmpd_a_b 
{
    public int[] a = new int[4];
    public float[,] b = new float [5,6];
}
"@

$dummy = New-H5LinkedDatatype type1 $t

cd group1

$dset = New-H5Dataset dset3 /type1 5

$value = New-Object 'cmpd_a_b[]' 5

foreach ($i in 0..4)
{
    $value[$i] = New-Object cmpd_a_b

    foreach ($j in 0..3) {
        $value[$i].a[$j] = $j
    }
    foreach ($r in 0..4) {
        foreach ($c in 0..5) {
            $value[$i].b[$r,$c] = ($r+1)/10
        }
    }
}

Set-H5DatasetValue dset3 $value

cd ..

$t = '{"Class": "Vlen", "Base": "H5T_STD_I32LE"}'

$dset = New-H5Dataset dset3 $t 4

$value = @(@(0), @(10,11), @(20,21,22), @(30,31,32,33))

Set-Content dset3 $value

$dummy = New-H5Hardlink group2 /group1

$dummy = New-H5Softlink slink1 somevalue

c:

Remove-H5Drive $name -Scope $scope
