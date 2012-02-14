
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
    [CmdletBinding(SupportsShouldProcess=$true)]
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
            if ($PSCmdlet.ShouldProcess($p, 'New HDF5 File'))
            { 
                try
                {
                    Write-Output(Format-H5File -Path $p)
                    Write-Host "`nSuccess: HDF5 file '$p' created."
                }
                catch
                {
                    Write-Debug ($_|Out-String)
                    Write-Error "`nCreation of HDF5 file '$p' failed."
                }
            }
        }
        else
        {
            Write-Error "`n'$p' file exists."
        }
    }
}
