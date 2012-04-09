
Function New-H5Softlink
{
<#
    .SYNOPSIS
      Creates a new HDF5 soft link
    .DESCRIPTION
      The New-H5Softlink function creates a new HDF5 soft link.
   .PARAMETER Source 
     The path name of the source
   .PARAMETER Destination 
     An HDF5 path name to the HDF5 destination object
   .EXAMPLE
     New-H5Softlink -Source h5:/group2 -Destination /group1
   .LINK
     New-Item
   .LINK
     http://www.hdfgroup.org/HDF5/doc/RM/RM_H5L.html#Link-CreateSoft
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The source path name.')]
        [string[]]
        $Source,
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=1,
                   HelpMessage='An HDF5 path name to the destination HDF5 object')]
        [string]
        $Destination
    )

   
    if ($PSCmdlet.ShouldProcess($Source, "New HDF5 Softlink -> '$Destination'"))
    {
        New-Item -Path $Source -ItemType SoftLink -Value $Destination
    }
}