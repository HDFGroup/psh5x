
$scope = 2

$ScriptDir = Split-Path $MyInvocation.MyCommand.Path

$files = dir $ScriptDir\..\..\h5\h5ex_t_*.h5

foreach ($f in $files)
{
    $drive = "$($f.Name.Substring(7,$f.Name.Length-10))"
    New-H5Drive $drive "$($f.FullName)" -Scope $scope
}

$names = @('array', 'bit', 'cmpd', 'enum', 'float', 'int', 'objref', 'opaque', 'regref', 'string', 'vlen', 'vlstring')

foreach ($n in $names)
{
    $n

    $dset = "$($n):\DS1"

    $dummy = Get-H5DatasetValue $dset 

    if (($n -ne 'objref') -and ($n -ne 'regref'))
    {
        Copy-H5Item $dset "h5tmp:\$n" -Force

        Set-H5DatasetValue "h5tmp:\$n" $dummy
    }
    else
    {
        Copy-H5Item "$($n):*" h5tmp:\

        Set-H5DatasetValue h5tmp:\DS1 $dummy

        if ($n -eq 'objref') {
            Remove-H5Item h5tmp:\DS1,h5tmp:\DS2
        }
    }
}

Remove-H5Item h5tmp:\*

c:

foreach ($f in $files)
{
    $drive = "$($f.Name.Substring(7,$f.Name.Length-10))"
    Remove-H5Drive $drive -Scope $scope
}
