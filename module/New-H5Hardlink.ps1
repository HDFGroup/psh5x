
Function New-H5Hardlink
{
<#
    .SYNOPSIS
      Creates a new HDF5 hard link
    .DESCRIPTION
      The New-H5Hardlink function creates a new HDF5 hard link.
    .PARAMETER Source 
      The path name of the source
    .PARAMETER Destination 
      An HDF5 path name to the HDF5 destination object
    .EXAMPLE
      New-H5Hardlink -Source h5:/group2 -Destination /group1
    .LINK
      New-Item
    .LINK
      http://www.hdfgroup.org/HDF5/doc/RM/RM_H5L.html#Link-CreateHard
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
        [string]
        $Destination
    )

    
    if ($PSCmdlet.ShouldProcess($Source, "New HDF5 Hardlink -> '$Destination'"))
    {
        New-Item -Path $Source -ItemType HardLink -Value $Destination
    }
}
