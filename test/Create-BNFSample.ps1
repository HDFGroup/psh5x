
$ScriptDir = Split-Path $MyInvocation.MyCommand.Path

$file = New-H5File $ScriptDir\sample.h5

$drive = New-H5Drive sample $ScriptDir\sample.h5 -RW

cd sample:\

$attr = New-H5Attribute . attr1 'string attribute' string17

$dset = New-H5Dataset dset1 H5T_STD_I32BE 10,10

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

$dset = New-H5Dataset dset2 $t 5 

$group = New-H5Group group1

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

$dtype = New-H5LinkedDatatype type1 $t

cd group1

$dset = New-H5Dataset dset3 /type1 5 

$t = '{"Class": "Vlen", "Base": "H5T_STD_I32LE"}'

cd ..

$dset = New-H5Dataset dset3 $t 4 

$hlink = New-H5Hardlink group2 /group1

$slink = New-H5Softlink slink1 somevalue

$logo = New-H5Image hdf_logo $ScriptDir\hdf_logo.jpg

e:
cd $ScriptDir

Remove-H5Drive sample

& 'C:\Program Files\TheHDFGroup\HDFView2.8\HDFView.exe' $ScriptDir\sample.h5
