
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope

cd "$($name):"

New-H5Dataset DS1 int 32,64 -Chunk 4,8 -Gzip 9

$value = New-Object 'int[,]' 32,64

foreach ($i in 0..31) {
    foreach ($j in 0..63) {
        $value[$i, $j] = $i*$j-$j
    }
}

Set-H5DatasetValue DS1 $Value

$max = (Get-Content DS1 | measure -Maximum | select Maximum)

if ($max.Maximum -ne 1890) {
    Write-Error "1890 != $max"
}

c:

Remove-H5Drive $name -Scope $scope 
