
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

New-H5Dataset DS2 H5T_STD_I8LE 3,2
$value = New-Object 'sbyte[]' 6
$value[0] = $value[1] = 1
$value[2] = $value[3] = 2
$value[4] = $value[5] = 3
Set-H5DatasetValue DS2 $value 

# create region references to the rows in the dataset and store them
# in another dataset

New-H5Dataset DS1 RegRef 3

$regref_t = 'System.Collections.Generic.KeyValuePair[String,Array]'
$value = @()

# reference to first row

$ref1 = New-Object 'System.UInt64[,]' 2,2
$ref1[0,0] = 0
$ref1[0,1] = 0
$ref1[1,0] = 0
$ref1[1,1] = 1
$a = New-Object $regref_t '/DS2',$ref1
$value += $a

# reference to second row

$ref2 = New-Object 'System.UInt64[,]' 2,2
$ref2[0,0] = 1
$ref2[0,1] = 0
$ref2[1,0] = 1
$ref2[1,1] = 1
$a = New-Object $regref_t '/DS2',$ref2
$value += $a

# reference to third row

$ref3 = New-Object 'System.UInt64[,]' 2,2
$ref3[0,0] = 2
$ref3[0,1] = 0
$ref3[1,0] = 2
$ref3[1,1] = 2
$a = New-Object $regref_t '/DS2',$ref3
$value += $a

Set-H5DatasetValue DS1 $value

$ref = Get-H5DatasetValue DS1 -Start 1

# no support for region references in Get-H5DatasetValue yet...

$start = $ref[0].Value[0,0],$ref[0].Value[0,1]
$block = ($ref[0].Value[1,0]-$ref[0].Value[0,0]+1),($ref[0].Value[1,1]-$ref[0].Value[0,1]+1)

$value = Get-H5DatasetValue "$($ref[0].Key)" -Start $start -Block $block

c:

Remove-H5Drive $name -Scope $scope 
