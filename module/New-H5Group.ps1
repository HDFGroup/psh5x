
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
   .PARAMETER TrackAttributeOrder
     Track the attribute creation order.
   .PARAMETER IndexAttributeOrder
     Maintain an index by attribute creation order.
   .PARAMETER TrackLinkOrder
     Track the link creation order.
   .PARAMETER IndexLinkOrder
     Maintain an index by link creation order.
   .PARAMETER Force
     Force the automatic creation of intermediate HDF5 groups.
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
   .LINK
     http://www.hdfgroup.org/HDF5/doc/RM/RM_H5G.html#Group-Create2
   .LINK
     http://www.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetAttrCreationOrder
   .LINK
     http://www.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetLinkCreationOrder
     
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The path of the new HDF5 group(s).')]
        [string[]]
        $Path,
        [Parameter(Mandatory=$false,
                   HelpMessage='Track attribute creation order?')]
        [switch]
        $TrackAttributeOrder,
        [Parameter(Mandatory=$false,
                   HelpMessage='Maintain an index by attribute creation order?')]
        [switch]
        $IndexAttributeOrder,
        [Parameter(Mandatory=$false,
                   HelpMessage='Track link creation order?')]
        [switch]
        $TrackLinkOrder,
        [Parameter(Mandatory=$false,
                   HelpMessage='Maintain an index by link creation order?')]
        [switch]
        $IndexLinkOrder,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    $cmd = 'New-Item -Path $Path -ItemType Group'
    
    if ($TrackAttributeOrder) {
        $cmd += ' -TrackAttributeOrder'
    }
    if ($IndexAttributeOrder) {
        $cmd += ' -IndexAttributeOrder'
    }
    if ($TrackLinkOrder) {
        $cmd += ' -TrackLinkOrder'
    }
    if ($IndexLinkOrder) {
        $param += ' -IndexLinkOrder'
    }
    if ($Force) {
        $param += ' -Force'
    }
    
    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 Group'))
    {
        Invoke-Expression $cmd            
    }
}
