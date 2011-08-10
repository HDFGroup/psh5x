
$t = @{
  Class    = 'String';
  CharSet  = 'Ascii';
  Length   = 17;
  StrPad   = 'Nullterm'
}

$ret = New-ItemProperty -Path h5tmp:\ -Name attr1 -ElementType $t

$ret = New-Item -Path h5tmp:\dset1 -ItemType Dataset `
    -ElementType H5T_STD_I32BE -Dimensions 10,10

$t = @{
  Class      = 'Compound';
  Size       = 16;
  Components = @{
    a = @{Class = 'Integer'; Type = 'H5T_STD_I32BE' ; `
          MemberOffset = 0 };
    b = @{Class = 'Float'  ; Type = 'H5T_IEEE_F32BE'; `
          MemberOffset = 4 };
    c = @{Type = 'H5T_IEEE_F64BE'; MemberOffset = 8 }
  }
}

$ret = New-Item -Path h5tmp:\dset2 -ItemType Dataset `
    -ElementType $t -Dimensions 5 

$ret = New-Item -Path h5tmp:\group1 -ItemType Group

$t = @{
  Class      = 'Compound';
  Size       = 136;
  Components = @{
    a = @{Class = 'Array'; Type = 'H5T_STD_I32BE' ; `
          Dims = @(4)  ; MemberOffset = 0 }
    b = @{Class = 'Array'; Type = 'H5T_IEEE_F32BE'; `
          Dims = @(5,6); MemberOffset = 16 }
  }
}

$ret = New-Item -Path h5tmp:\type1 -ItemType Datatype -Definition $t

$ret = New-Item -Path h5tmp:\group1\dset3 -ItemType Dataset `
    -ElementType /type1 -Dimensions 5 

$t = @{Class = 'Vlen'; Type = 'H5T_STD_I32LE'}

$ret = New-Item -Path h5tmp:\dset3 -ItemType Dataset `
    -ElementType $t -Dimensions 4 

$ret = New-Item -Path h5tmp:\group2 -ItemType HardLink -Value /group1

$ret = New-Item -Path h5tmp:\slink1 -ItemType SoftLink -Value somevalue

