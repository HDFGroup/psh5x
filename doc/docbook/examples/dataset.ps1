Add-PSSnapin HDF5

New-PSDrive -Name 'sample1' -PSProvider HDF5 -Root sample1:\ `
    -Path .\sample1.h5 -Force

New-Item -Path sample1:\floats -ItemType Dataset -ElementType Double `
    -Dimensions 256,512 -MaxDimensions -1,-1 -Chunked 32,64 -Gzip 7

$data = New-Object 'double[,]' 256,512

foreach ($i in 0..255)
{
    $data[$i, $i] = $i;
    $data[$i, (256+$i)] = $i;
}

Set-Content sample1:\floats $data

Remove-PSDrive sample1
