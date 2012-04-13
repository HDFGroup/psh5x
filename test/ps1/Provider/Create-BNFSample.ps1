
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

$dset = New-Item dset1 -ItemType Dataset -ElementType H5T_STD_I32BE -Dimensions 10,10

$value = New-Object 'int[,]' 10,10

foreach ($i in 0..9) {
    foreach ($j in 0..9) {
        $value[$i,$j] = $j
    }
}

Set-Content dset1 $value
$a = Get-Content dset1 -ReadCount 0

foreach ($i in 0..9) {
    foreach ($j in 0..9) {
        if ($value[$i,$j] -ne $a[$i, $j]) {
            Write-Error "$($value[$i,$j]) != $($a[$i, $j])"
        }
    }
}

$a = Get-Content dset1 -ReadCount 5

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

$dset = New-Item dset2 -ItemType Dataset -ElementType $t -Dimensions 5 

$value = New-Object 'cmpd_a_b_c[]' 5

foreach ($i in 0..4)
{
    $value[$i] = New-Object cmpd_a_b_c

    $value[$i].a = $i+1
    $value[$i].b = ($i+1)/10
    $value[$i].c = ($i+1)/100
}

Set-Content dset2 $value
$a = Get-Content dset2 -ReadCount 0

foreach ($i in 0..4)
{
    if ($value[$i].a -ne $a[$i].i0) {
        Write-Error "$($value[$i].a) != $($a[$i].i0)"
    }
    if ($value[$i].b -ne $a[$i].f1) {
        Write-Error "$($value[$i].b) != $($a[$i].f1)"
    }
    if ($value[$i].c -ne $a[$i].d2) {
        Write-Error "$($value[$i].c) != $($a[$i].d2)"
    }
}

$a = Get-Content dset2 -ReadCount 2

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

Add-Type @"
public class cmpd_a_b 
{
    public int[] a = new int[4];
    public float[,] b = new float [5,6];
}
"@

New-Item type1 -ItemType Datatype -Definition $t

cd group1

$dset = New-Item dset3 -ItemType Dataset -ElementType /type1 -Dimensions 5 

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

Set-Content dset3 $value
$a = Get-Content dset3 -ReadCount 0

foreach ($i in 0..4)
{
    foreach ($j in 0..3) {
        if ($value[$i].a[$j] -ne $a[$i].A0[$j]) {
            Write-Error "$($value[$i].a[$j]) != $($a[$i].A0[$j])"
        }
    }
    foreach ($r in 0..4) {
        foreach ($c in 0..5) {
            if ($value[$i].b[$r,$c] -ne $a[$i].A1[$r,$c]) {
                Write-Error "$($value[$i].b[$r,$c]) != $($a[$i].A1[$r,$c])"
            }
        }
    }
}

$a = Get-Content dset3 -ReadCount 2

cd ..

$t = '{"Class": "Vlen", "Base": "H5T_STD_I32LE"}'

New-Item dset3 -ItemType Dataset -ElementType $t -Dimensions 4 

$value = (,0), (10,11), (20,21,22), (30,31,32,33)

Set-Content dset3 $value
$a = Get-Content dset3 -ReadCount 0

foreach ($i in 0..3)
{
    if ($value[$i].Count -ne $a[$i].Count) {
        Write-Error "$($value[$i].Count) != $($a[$i].Count)"
    }

    foreach ($j in 0..$($value[$i].Count)) {
        if ($value[$i][$j] -ne $a[$i][$j]) {
            Write-Error "$($value[$i][$j]) != $($a[$i][$j])"
        }
    }
}

$a = Get-Content dset3 -ReadCount 2

New-Item group2 -ItemType Hardlink -Value /group1

New-Item slink1 -ItemType Softlink -Value somevalue 

c:

Remove-PSDrive $name -Scope $scope 
