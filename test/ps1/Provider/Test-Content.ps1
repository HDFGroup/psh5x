
$scope = 2

$ScriptDir = Split-Path $MyInvocation.MyCommand.Path

$files = dir $ScriptDir\..\..\h5\h5ex_t_*.h5

foreach ($f in $files)
{
    $drive = "$($f.Name.Substring(7,$f.Name.Length-10))"
    New-PSDrive $drive HDF5 "$($drive):\"  -Path "$($f.FullName)" -Scope $scope
}

$names = @('array', 'bit', 'cmpd', 'enum', 'float', 'int', 'objref', 'opaque', 'regref', 'string', 'vlen', 'vlstring')

foreach ($n in $names)
{
    $n

    $dset = "$($n):\DS1"

    $dummy = Get-Content $dset 
    $dummy = Get-Content $dset -ReadCount 0

    if (($n -ne 'objref') -and ($n -ne 'regref'))
    {
        Copy-Item $dset "h5tmp:\$n" -Force

        Set-Content "h5tmp:\$n" $dummy
    }
    else
    {
        Copy-Item "$($n):*" h5tmp:\

        Set-Content h5tmp:\DS1 $dummy

        if ($n -eq 'objref') {
            Remove-Item h5tmp:\DS1,h5tmp:\DS2
        }
    }
}

Remove-Item h5tmp:\*

c:

foreach ($f in $files)
{
    $drive = "$($f.Name.Substring(7,$f.Name.Length-10))"
    Remove-PSDrive $drive -Scope $scope
}
