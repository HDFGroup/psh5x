
Function Format-H5Dataset
{
<#
    .SYNOPSIS
      Reshapes the extent of extendible HDF5 datasets
    .PARAMETER Path
      The path(s) to extendible HDF5 dataset(s)
    .PARAMETER Dimensions 
      The new extent of the HDF5 dataset(s)
    .PARAMETER PassThru
      Return a PowerShell object representing each updated HDF5 dataset.
    .LINK
      New-H5Dataset
    .LINK
      http://www.hdfgroup.org/HDF5/doc/RM/RM_H5S.html#Dataspace-SetExtentSimple
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   Position=0,
                   ValueFromPipelineByPropertyName=$true,
                   HelpMessage='The path to one or more extendible HDF5 datasets')]
        [ValidateNotNull()]
        [string[]]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The new extent of the HDF5 dataset(s)')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [uint64[]]
        $Dimensions,
        [Parameter(Mandatory=$false,
                   HelpMessage='Write objects to pipeline?')]
        [switch]
        $PassThru
    )
    
    $cmd = 'Set-Item -Path $Path -Value $Dimensions'
    
    if ($PassThru) {
        $cmd += ' -PassThru'
    }
    
    if ($PSCmdlet.ShouldProcess($Path, 'Re-shaping HDF5 dataset'))
    {       
        Invoke-Expression $cmd
    }
}
