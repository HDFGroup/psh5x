
$t = @{
  Class    = 'String';
  CharSet  = 'Ascii';
  Length   = 17;
  StrPad   = 'Nullterm'
}

New-ItemProperty -Path h5tmp:\ -Name attr1 -ElementType $t

New-Item -Path h5tmp:\dset1 -ItemType Dataset -ElementType H5T_STD_I32BE -Dimensions 10,10

$t = @{
  Class      = 'Compound';
  Size       = 16;
  Components = @{
    a = @{Class = 'Integer'; Type = 'H5T_STD_I32BE' ; MemberOffset = 0 };
    b = @{Class = 'Float'  ; Type = 'H5T_IEEE_F32BE'; MemberOffset = 4 };
    # for known types we don't have to specify the type class
    c = @{Type = 'H5T_IEEE_F64BE'; MemberOffset = 8 }
  }
}

New-Item -Path h5tmp:\dset2 -ItemType Dataset -ElementType $t -Dimensions 5 

New-Item -Path h5tmp:\group1 -ItemType Group

$t = @{
  Class      = 'Compound';
  Size       = 136;
  Components = @{
    a = @{Class = 'Array'; Type = 'H5T_STD_I32BE' ; Dims = @(4)  ; MemberOffset = 0 }
    b = @{Class = 'Array'; Type = 'H5T_IEEE_F32BE'; Dims = @(5,6); MemberOffset = 16 }
  }
}

New-Item -Path h5tmp:\type1 -ItemType Datatype -Definition $t

New-Item -Path h5tmp:\group1\dset3 -ItemType Dataset -ElementType /type1 -Dimensions 5 

$t = @{Class = 'Vlen'; Type = 'H5T_STD_I32LE'}

New-Item -Path h5tmp:\dset3 -ItemType Dataset -ElementType $t -Dimensions 4 

New-Item -Path h5tmp:\group2 -ItemType HardLink -Value /group1

New-Item -Path h5tmp:\slink1 -ItemType SoftLink -Value somevalue

& 'C:\Program Files\TheHDFGroup\HDFView2.7\HDFView.exe' $Env:PSH5XTmpFile
