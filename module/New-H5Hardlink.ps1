
Function New-H5Hardlink
{
<#
    .SYNOPSIS
      Creates a new HDF5 hardlink ("adds a group member")
    .DESCRIPTION
      The New-H5Hardlink function creates a new HDF5 hardlink.
      Unless the current location is on the targeted H5Drive,
      the source path name must be drive-qualified.
   .PARAMETER Source 
     The path name of the source
   .PARAMETER Destination 
     An HDF5 path name to the HDF5 destination object
   .EXAMPLE
     New-H5Hardlink -Source h5:/group2 -Destination /group1
   .LINK
     New-Item
   .NOTES
     Forward- (/) and backslash (\) seprators are supported in path names.
     They MUST NOT be used as part of link names.
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   HelpMessage='The source path name.')]
        [string]
        $Source,
        [Parameter(Mandatory=$true,
                   HelpMessage='A path to the destination HDF5 object')]
        [string]
        $Destination
    )

    if ((Test-Path $Source))
    {
        Write-Error "`nThe source path '$Destination' is in use."
        return
    }
    if (!(Test-Path $Destination))
    {
        Write-Error "`nThe destination '$Destination' is invalid."
        return
    }

    if ($PSCmdlet.ShouldProcess($Source, "New HDF5 Hardlink -> '$Destination'"))
    { 
        try
        {
            Write-Output(
                New-Item -Path $Source -ItemType HardLink -Value $Destination)
            Write-Host "`nSuccess: HDF5 hardlink '$Source' -> '$Destination' created."
        }
        catch {
            Write-Debug($_|Out-String)
            Write-Error "`nUnable to create HDF5 hardlink '$Source'."
        }
    }
}
