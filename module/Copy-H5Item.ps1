
Function Copy-H5Item
{
<#
    .SYNOPSIS
      Copies one or more HDF5 items
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

      The source and destination may refer to different H5Drives.
    .PARAMETER Source
      The path of the HDF5 object to be copied.
    .PARAMETER Destination
      The path of the new HDF5 object.
    .PARAMETER Recurse 
      For HDF5 groups, recursively copy linked HDF5 objects.
    .PARAMETER IgnoreAttributes 
      Ignore all HDF5 attributes of the object to be copied.
    .PARAMETER ExpandSoftLinks
      Expand soft links into new objects rather than copying the link
    .PARAMETER ExpandExternalLinks
      Expand external links into new objects rather than copying the link
    .PARAMETER ExpandReferences
      Copy objects referenced by object references.
    .PARAMETER Force
      Force the creation of intermediaries.
    .PARAMETER PassThru
      Return an object representing each copied item.
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
                   ValueFromPipeline=$true,
                   Position=0,
                   HelpMessage='The path to the HDF5 object to be copied.')]
        [ValidateNotNull()]
        [string[]]
        $Path,
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
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
                   HelpMessage='Expand soft links?')]
        [switch]
        $ExpandSoftLinks,
        [Parameter(Mandatory=$false,
                   HelpMessage='Expand external links?')]
        [switch]
        $ExpandExternalLinks,
        [Parameter(Mandatory=$false,
                   HelpMessage='Expand object references?')]
        [switch]
        $ExpandObjectReferences,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediaries?')]
        [switch]
        $Force,
        [Parameter(Mandatory=$false,
                   HelpMessage='Write objects to pipeline?')]
        [switch]
        $PassThru
    )

    if ($PSCmdlet.ShouldProcess($Path, 'Copy HDF5 object'))
    { 
        $cmd = "Copy-Item $Path $Destination"
        
        if ($Recurse) {
            $cmd = "$cmd -Recurse"
        }
        if ($IgnoreAttributes) {
            $cmd = "$cmd -IgnoreAttributes"
        }
        if ($ExpandSoftLinks) {
            $cmd = "$cmd -ExpandSoftLinks"
        }
        if ($ExpandExternalLinks) {
            $cmd = "$cmd -ExpandExternalLinks"
        }
        if ($ExpandObjectReferences) {
            $cmd = "$cmd -ExpandObjectReferences"
        }
        if ($Force) {
            $cmd = "$cmd -Force"
        }
        if ($PassThru) {
            $cmd = "$cmd -PassThru"
        }
        
        Invoke-Expression $cmd
    }
}
