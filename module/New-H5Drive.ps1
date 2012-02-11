
Function New-H5Drive(
                   [string] $name=($paramMissing=$true),
                   [string] $file=($paramMissing=$true),
                   [switch] $rw,
                   [string] $root
                   )
{
<#
    .SYNOPSIS
      Create a new HDF5 Drive 
    .DESCRIPTION
      The New-H5Drive function creates a new PSDrive backed by the specified
      HDF5 file. It returns an error if the drive name is in use, the file
      doesn't exist, or the user has insufficient permissions.
    .EXAMPLE
      New-H5Drive -name h5 -file C:\tmp\foo.h5
 #>
    If($local:paramMissing)
    {
        throw "USAGE: New-H5Drive -name <drive name> -file <file path> [-rw] [-root <root path>]"
    }
    If (!$root)
    {
        $root = "$($name):\"
    }
    If ($rw)
    {
        "`nAttempting to create new writeable HDF5 drive $name with file $file, and root $root ..."
        $status = (New-PSDrive -Name $name -PSProvider HDF5 -Path $file -Root $root -Mode RW -Scope 2)
    }
    else
    {
        "`nAttempting to create new read-only HDF5 drive $name with file $file, and root $root ..."
        $status = (New-PSDrive -Name $name -PSProvider HDF5 -Path $file -Root $root -Scope 2)
    }
    $count = 0
    Get-PSDrive -PSProvider HDF5 -Scope 1 | ?{$_.Name -eq $name} | %{ $count++ }
    If ($count -eq 1)
    {
        "`nSuccess: HDF5 drive $name created."
        return $status
    }
    Else
    {
        "`nError: Creation of HDF5 drive $name failed."
    }
}
