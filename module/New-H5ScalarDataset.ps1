
Function New-H5ScalarDataset
{
<#
    .SYNOPSIS
      Creates a new HDF5 scalar dataset (singleton)
    .DESCRIPTION
      The New-H5ScalarDataset function creates a new HDF5 scalar dataset.
      Use the -Force switch to automatically create missing intermediate
      groups. Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .PARAMETER Path
     The path of the new HDF5 Dataset.
   .PARAMETER Type
     The element type of the HDF5 Dataset. The type can be specified as
     1) A pre-defined HDF5 datatype (string)
     2) An HDF5 datatype definition (JSON)
     3) The HDF5 path name of a linked HDF5 datatype (string)
   .PARAMETER Compact 
     Create an HDF5 dataset with compact layout.
   .PARAMETER Force
     Force the automatic creation of intermediate HDF5 groups.
   .EXAMPLE
   .LINK
     New-Item
   .LINK
      New-H5Dataset
   .LINK
      http://www.hdfgroup.org/HDF5/doc/RM/RM_H5S.html#Dataspace-Create
 #>
    [CmdletBinding(SupportsShouldProcess=$true,
                   DefaultParametersetName='Simple')]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The path to the new HDF5 dataset.')]
        [string[]]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The element type of the new scalar HDF5 dataset.')]
        [ValidateNotNull()]
        [string]
        $Type,
        [Parameter(Mandatory=$false,
                   HelpMessage='Compact layout?')]
        [switch]
        $Compact,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    $cmd = 'New-Item -Path $Path -ItemType Dataset -ElementType $Type -Scalar'
    
    if ($Compact) {
        $cmd += ' -Compact'
    }
    if ($Force) {
        $cmd += ' -Force'
    }

    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 Null Dataset'))
    { 
        Invoke-Expression $cmd
    }
}