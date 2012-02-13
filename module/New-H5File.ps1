
Function New-H5File
{
<#
    .SYNOPSIS
      Creates new HDF5 files
    .DESCRIPTION
      The New-H5File function creates new HDF5 files at the specified locations.
      It returns an error if a file or directory exists or the user does not 
      have sufficient permissions.
    .PARAMETER Path
      The path name(s) of the new HDF5 file(s).
    .EXAMPLE
      New-H5File -Path C:\tmp\foo.h5
    .EXAMPLE
      New-H5File C:\tmp\foo.h5,C:\tmp\bar.h5
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   HelpMessage='The path name of the new HDF5 file.')]
        [string[]]
        $Path
    )

    foreach ($p in $Path)
    {
        if(!(Test-Path $p))
        {
            try
            {
                $file = (Format-H5File -Path $p)
                Write-Host "`nSuccess: HDF5 file '$p' created."
                Write-Output $file
            }
            catch
            {
                Write-Debug ($_|Out-String)
                Write-Host "`nError: creation of HDF5 file '$p' failed."
            }
        }
        else
        {
            Write-Host "`nError: '$p' file exists."
        }
    }
}
