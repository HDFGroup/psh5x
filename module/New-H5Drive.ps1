
Function New-H5Drive(
                   [string] $name=($paramMissing=$true),
                   [string] $path=($paramMissing=$true),
                   [switch] $rw,
                   [string] $root
                   )
{
<#
    .SYNOPSIS
      Create a new HDF5 Drive 
    .EXAMPLE
      New-H5Drive -name h5 -path C:\tmp\foo.h5
 #>
    If($local:paramMissing)
    {
        throw "USAGE: New-H5Drive -name <drive name> -path <file path> [-rw] [-root <root path>]"
    }
    If (!$root)
    {
        $root = "$($name):\"
    }
    If ($rw)
    {
        "`nAttempting to create new writeable HDF5 drive $name with file $path, and root $root ..."
    }
    else
    {
        "`nAttempting to create new read-only HDF5 drive $name with file $path , and root $root ..."
    }

    New-PSDrive -Name $name -PSProvider HDF5 -Path $path
}
