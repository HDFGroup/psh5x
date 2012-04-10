
Function New-H5PacketTable
{
<#
    .SYNOPSIS
      Creates a new HDF5 packet table 
    .DESCRIPTION
      The New-H5PacketTable function creates a new HDF5 packet table.
   .PARAMETER Path
     The path of the new HDF5 packet table.
   .PARAMETER PacketType
     The packet type of the HDF5 packet table. The type can be specified as
     1) A pre-defined HDF5 datatype
     2) An HDF5 datatype definition (JSON)
     3) The HDF5 path name of a linked HDF5 datatype
   .PARAMETER ChunkSize 
     Create an HDF5 packet table with the specified chunk size.
   .PARAMETER Gzip 
     Create an HDF5 packet table with gzip compression enabled.
   .PARAMETER Force
     Force the automatic creation of intermediate HDF5 groups.
   .EXAMPLE
   .LINK
     New-Item
   .LINK
     http://www.hdfgroup.org/HDF5/doc/HL/RM_H5PT.html#H5PTcreate_fl
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The path to the new HDF5 packet table.')]
        [ValidateNotNull()]
        [string[]]
        $Path,
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=1,
                   HelpMessage='The packet type of the new HDF5 packet table.')]
        [ValidateNotNull()]
        [string]
        $PacketType,
        [Parameter(Mandatory=$true,
                   Position=2,
                   HelpMessage='Chunk size in bytes?')]
        [ValidateScript({$_ -gt 0})]
        [uint32]
        $ChunkSize,
        [Parameter(Mandatory=$false,
                   Position=3,
                   HelpMessage='Gzip level ([-1,9])')]
        [ValidateRange(-1,9)]
        [int]
        $Gzip,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    $cmd = 'New-Item -Path $Path -ItemType PacketTable -PacketType $PacketType -ChunkSize $ChunkSize'
        
    if ($Force) {
        $cmd += ' -Force'
    }
        
    if ($Gzip) {
      $cmd += ' -Gzip $Gzip'
    }
        
    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 Packet Table'))
    {     
        Invoke-Expression $cmd
    }
}
