
Function New-H5Group
{
<#
    .SYNOPSIS
      Create a new HDF5 group 
    .DESCRIPTION
      The New-H5Group function creates one or more new HDF5 groups.
      Use the -Force switch to automatically create missing intermediate
      groups. Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .PARAMETER Path
     The path of the new HDF5 group(s).
   .PARAMETER Force
     Force the creation of intermediates.
   .EXAMPLE
     New-H5Group -Path h5:\group1
   .EXAMPLE
     New-H5Group -Path h5:\group2\group3 -Force
   .EXAMPLE
     New-H5Group -Path h5:/group1
   .EXAMPLE
     New-H5Group -Path h5:/group1/group2
   .LINK
     New-Item
   .NOTES
     Forward- (/) and backslash (\) seprators are supported in path names.
     The must not be used as part of link names.
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   HelpMessage='The path of the new HDF5 group(s).')]
        [string[]]
        $Path,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    foreach ($p in $Path)
    {
        if (!(Test-Path $p))
        {
            if ($PSCmdlet.ShouldProcess($p, 'New HDF5 Group'))
            { 
                try
                {
                    if ($Force)
                    {
                        Write-Output(New-Item -Path $p -ItemType Group -Force)
                        Write-Host "`nSuccess: HDF5 group '$p' created."
                    }
                    else
                    {
                        Write-Output(New-Item -Path $p -ItemType Group)
                        Write-Host "`nSuccess: HDF5 group '$p' created."
                    }
                }
                catch {
                    Write-Debug($_|Out-String)
                    Write-Error "`nUnable to create HDF5 group '$p'. Intermediates missing? Is your current location an H5Drive?"
                }
            }
        }
        else
        {
            Write-Error "`nPath name '$p' is in use."
        }
    }
}
