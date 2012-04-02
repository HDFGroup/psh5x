
Function Copy-H5SymbolicLink
{
<#
    .SYNOPSIS
      Copies an HDF5 symbolic link 
    .DESCRIPTION
      The Copy-H5SymbolicLink function creates a copy of an existing
      HDF5 symbolic link (soft- or external link).

      Use the -Force switch to automatically create missing intermediate groups.

      Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.

      The source and destination may refer to different H5Drives.
    .PARAMETER Source
      The path of the HDF5 symbolic link to be copied.
    .PARAMETER Destination
      The path name of the new HDF5 symbolic link.
    .PARAMETER Force
      Force the creation of intermediaries.
    .EXAMPLE
    .LINK
      Copy-Item
    .NOTES
      Forward- (/) and backslash (\) seprators are supported in path names.
      They must not be used as part of link names.
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   Position=0,
                   HelpMessage='The path to the HDF5 object to be copied.')]
        [ValidateNotNull()]
        [string]
        $Source,
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The path to the destination.')]
        [ValidateNotNull()]
        [string]
        $Destination,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediaries?')]
        [switch]
        $Force
    )

    if (!(Test-Path $Source))
    {
        Write-Error "`nSource doesn't exist."
        return
    }
    else
    {
        $item_type = (Get-Item $Source).ItemType.ToUpper()
        if (($item_type -ne 'SOFTLINK') -and ($item_type -ne 'EXTERNALLINK'))
        {
            Write-Error "`nSource is not a symbolic link."
            return
        }
    }

    if ($PSCmdlet.ShouldProcess($Path, 'Copy HDF5 symbolic link'))
    { 
        $param = @("'$Source'", "'$Destination'", '-CopyLink')
    
        if ($Force) {
            $param += '-Force'
        }
  
        Write-Output(Invoke-Expression "Copy-Item $param")
    
        if (Test-Path $Destination) {
            Write-Host "`nSuccess: HDF5 object '$Source' copied to '$Destination'."
        }
    }
}
