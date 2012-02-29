
Function Copy-H5Object
{
<#
    .SYNOPSIS
      Copies an HDF5 object 
    .DESCRIPTION
      The Copy-H5Object function creates a copy of an existing HDF5 object.
      The default behaviour is to include decorating HDF5 attributes and
      to not recursively copy HDF5 groups.

      By default, for HDF5 groups, only objects linked to the group are
      copied. Use the -Recurse switch to recusively follow links and also
      copy object that are not directly linked to this group.

      Use the -IgnoreAttributes switch to prevent copying the HDF5 attributes
      of the object(s) to be copied.

      Use the -Force switch to automatically create missing intermediate groups.

      Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .PARAMETER Source
     The path of the HDF5 object to be copied.
   .PARAMETER Destination
     The path of the new HDF5 object.
   .PARAMETER Recurse 
     For HDF5 groups, recursively copy linked HDF5 objects.
   .PARAMETER IgnoreAttributes 
     Ignore all HDF5 attributes of the object to be copied.
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
                   HelpMessage='Recurse?')]
        [switch]
        $Recurse,
        [Parameter(Mandatory=$false,
                   HelpMessage='Ignore attributes?')]
        [switch]
        $IgnoreAttributes,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediaries?')]
        [switch]
        $Force
    )

    if (!(Test-Path $Source))
    {
        Write-Error "`nThe source path name '$Source' cannot be resolved."
        return
    }
    if (Test-Path $Destination)
    {
        Write-Error "`nThe destination path name '$Destination' is in use."
        return
    }

    if ($PSCmdlet.ShouldProcess($Path, 'Copy HDF5 object'))
    { 
        $param = @($Source, $Destination)

        if ($Force) {
            $param += '-Force'
        }
        if ($IgnoreAttributes) {
            $param += '-IgnoreAttributes'
        }
        if ($Recurse) {
            $param += '-Recurse'
        }

        Write-Output(Invoke-Expression "Copy-Item $param")

        if (Test-Path $Destination) {
            Write-Host "`nSuccess: HDF5 object '$Source' copied to '$Destination'."
        }
    }
}
