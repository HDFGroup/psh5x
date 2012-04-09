
Function Copy-H5Item
{
<#
    .SYNOPSIS
      Copies one or more HDF5 items
    .DESCRIPTION
      The Copy-H5Item function creates a copy of existing HDF5 items.
      The default behaviour is to include decorating HDF5 attributes and
      NOT to recursively copy HDF5 groups. That means that for HDF5 groups
      only items linked to the group are copied. Use the -Recurse switch to
      recusively follow links and also copy items that are not directly linked
      to this group.
      
      HDF5 items can be copied between different H5Drives.

      Use the -IgnoreAttributes switch to suppress copying the HDF5 attributes
      of the object(s) to be copied.

      Use the -ExpandSoftLinks switch to expand soft links instead of copying
      them as they are.

      Use the -ExpandExternalLinks switch to expand external links instead
      of copying them as they are.
      
      Use the -ExpandReferences switch to copy objetcs referenced instead of
      updating the reference values.

      Use the -Force switch to automatically create missing intermediate groups.
    .PARAMETER Source
      The path of the HDF5 item(s) to be copied.
    .PARAMETER Destination
      The destination of the new HDF5 item(s).
    .PARAMETER Recurse 
      For HDF5 groups, recursively copy linked HDF5 items.
    .PARAMETER IgnoreAttributes 
      Suppress copying HDF5 attributes of the object(s) to be copied.
    .PARAMETER ExpandSoftLinks
      Expand HDF5 soft links into new objects rather than copying them.
    .PARAMETER ExpandExternalLinks
      Expand HDF5 external links into new objects rather than copying them.
    .PARAMETER ExpandObjectReferences
      Copy HDF5 objects referenced by object references instead of updating them.
    .PARAMETER Force
      Force the automatic creation of intermediate HDF5 groups.
    .PARAMETER PassThru
      Return a PowerShell object representing each copied HDF5 item.
    .EXAMPLE
    .LINK
      Copy-Item
    .LINK
      http://www.hdfgroup.org/HDF5/doc/RM/RM_H5O.html#Object-Copy
    .LINK
      http://www.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetCopyObject
    .NOTES
      When copying HDF5 items recursively and the -PassThru switch is present
      only PowerShell objects representing the top-level HDF5 groups are returned.
      
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipeline=$true,
                   Position=0,
                   HelpMessage='The path to the HDF5 object(s) to be copied.')]
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
    
    $cmd = 'Copy-Item $Path $Destination'
        
    if ($Recurse) {
        $cmd += ' -Recurse'
    }
    if ($IgnoreAttributes) {
        $cmd += ' -IgnoreAttributes'
    }
    if ($ExpandSoftLinks) {
        $cmd += ' -ExpandSoftLinks'
    }
    if ($ExpandExternalLinks) {
        $cmd += ' -ExpandExternalLinks'
    }
    if ($ExpandObjectReferences) {
       $cmd += ' -ExpandObjectReferences'
    }
    if ($Force) {
       $cmd += ' -Force'
    }
    if ($PassThru) {
       $cmd += ' -PassThru'
    }

    if ($PSCmdlet.ShouldProcess($Path, 'Copy HDF5 object'))
    {       
        Invoke-Expression $cmd
    }
}
