
Function New-H5File(
                   $path=($paramMissing=$true)
                   )
{
<#
    .SYNOPSIS
      Create a new HDF5 file
    .EXAMPLE
      New-H5File -path C:\tmp\foo.h5
 #>
    If($local:paramMissing)
    {
        throw "USAGE: New-H5File -path <file path>"
    } #$local:paramMissing
    "`nAttempting to create new HDF5 file $path ..."
}
