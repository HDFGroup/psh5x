
Function New-H5File
{
<#
    .SYNOPSIS
      Create a new HDF5 file
    .DESCRIPTION
      The New-H5File function creates a new HDF5 at the specified location.
      It returns an error if a file or directory exists at the specified
      location, or the user does not have write permissions.
    .EXAMPLE
      New-H5File -Path C:\tmp\foo.h5
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipeline=$false,
                   HelpMessage='The path name of the new HDF5 file.')]
        [string]
        $Path
    )

    "`nAttempting to create new HDF5 file '$Path' ..."

    if(!(Test-Path $path))
    {
        $file = (Format-H5File -Path $Path)
        $status = (Test-H5File $Path)
        if ($status)
        {
            "`nSuccess: HDF5 file '$Path' created."
            $file
        }
        else
        {
            throw "`nError: file creation failed."
        }
    }
    else
    {
        throw "`nError: $Path file exists."
    }
}
