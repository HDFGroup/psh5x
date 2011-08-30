### create the root group's attr1 attribute

$ret = New-ItemProperty -Path h5tmp:\ -Name attr1 -ElementType string17

### create and populate /dset1

$ret = New-Item -Path h5tmp:\dset1 -ItemType Dataset `
    -ElementType H5T_STD_I32BE -Dimensions 10,10

$a = New-Object 'int[]' 100

foreach ($i in 0..99) { $a[$i] = $i%10 }

Set-Content h5tmp:\dset1 $a

### create and populate /dset2

$t = @{
  Class   = 'Compound';
  Size    = 16;
  Members = @{
    a = @{Class = 'Integer'; Type = 'H5T_STD_I32BE' ; MemberOffset = 0 };
    b = @{Class = 'Float'  ; Type = 'H5T_IEEE_F32BE'; MemberOffset = 4 };
    c = @{Class = 'Float'  ; Type = 'H5T_IEEE_F64BE'; MemberOffset = 8 }
  }
}

$ret = New-Item -Path h5tmp:\dset2 -ItemType Dataset `
    -ElementType $t -Dimensions 5

$a = New-H5Array -Type $t -Dimensions 5

foreach ($i in 1..5)
{
  $a[$i-1].i2 = $i; $a[$i-1].f1 = $i/10; $a[$i-1].d0 = $i/100;
}

Set-Content h5tmp:\dset1 $a

### create group /group1

$ret = New-Item -Path h5tmp:\group1 -ItemType Group

### create named datatype /type1

$t = @{
  Class   = 'Compound';
  Size    = 136;
  Members = @{
    a = @{Class = 'Array'; Type = 'H5T_STD_I32BE' ; `
          Dims = @(4)  ; MemberOffset = 0 }
    b = @{Class = 'Array'; Type = 'H5T_IEEE_F32BE'; `
          Dims = @(5,6); MemberOffset = 16 }
  }
}

$ret = New-Item -Path h5tmp:\type1 -ItemType Datatype -Definition $t

### create and populate /group1/dset3

$ret = New-Item -Path h5tmp:\group1\dset3 -ItemType Dataset `
    -ElementType /type1 -Dimensions 5 

$a = New-H5Array -Type $t -Dimensions 5

$x = @(0, 1, 2, 3)
$y = New-Object 'float[,]' 5,6

foreach ($i in 1..5) { foreach ($j in 0..5) { $y[$i-1, $j] = $i/10 } }

foreach ($i in 1..5)
{
  $a[$i-1].ai0 = $x
  $a[$i-1].af0 = $y
}

Set-Content h5tmp:\group1\dset3 $a

### create and populate /dset3

$t = @{Class = 'Vlen'; Type = 'H5T_STD_I32LE'}

$ret = New-Item -Path h5tmp:\dset3 -ItemType Dataset `
    -ElementType $t -Dimensions 4 

$a = @(@(0), @(10, 11), @(20, 21, 22), @(30, 31, 32, 33))

Set-Content h5tmp:\dset3 $a

### create hard link /group2 -> /group1

$ret = New-Item -Path h5tmp:\group2 -ItemType HardLink -Value /group1

### create soft link /slink1

$ret = New-Item -Path h5tmp:\slink1 -ItemType SoftLink -Value somevalue
