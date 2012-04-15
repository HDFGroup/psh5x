
Function New-H5Extlink
{
<#
    .SYNOPSIS
      Creates a new HDF5 external link
    .DESCRIPTION
      The New-H5Externallink function creates a new HDF5 external link.
    .PARAMETER Source 
      The path name of the source
    .PARAMETER Destination 
      A pair of an HDF5 file name and an HDF5 path name
    .EXAMPLE
      New-H5Extlink -Source h5:/ext1 -Destination my.h5,/group1
    .LINK
      New-Item
    .LINK
      http://www.hdfgroup.org/HDF5/doc/RM/RM_H5L.html#Link-CreateExternal
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The source path name.')]
        [ValidateNotNull()]
        [string[]]
        $Source,
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=1,
                   HelpMessage='An HDF5 path name to the destination HDF5 object')]
        [ValidateNotNull()]
        [ValidateCount(2,2)]
        [string[]]
        $Destination
    )

    if ($PSCmdlet.ShouldProcess($Source, "New HDF5 Externallink -> '$Destination'"))
    {
        New-Item -Path $Source -ItemType ExtLink -Value $Destination
    }
}
