
Function New-H5File(
                   [string] $path=($paramMissing=$true)
                   )
{
<#
    .SYNOPSIS
      Create a new HDF5 file
    .DESCRIPTION
      The New-H5File function creates a new HDF5 at the specified location.
      It returns an error if a file or directory exists at the specified
      location, or the user does not have write permissions.
    .EXAMPLE
      New-H5File -path C:\tmp\foo.h5
 #>
    If($local:paramMissing)
    {
        throw "USAGE: New-H5File -path <file path>"
    }
    "`nAttempting to create new HDF5 file $path ..."
    If(!(Test-Path $path))
    {
        $file = (Format-H5File -Path $path)
        $status = (Test-H5File $path)
        If ($status)
        {
            Write-Host "`nSuccess: HDF5 file $path created."
            return $file
        }
        Else
        {
            Write-Error "`nError: file creation failed."
        }
    }
    Else
    {
        Write-Error "`nError: $path file exists."
    }
}
