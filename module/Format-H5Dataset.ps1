
Function Format-H5Dataset
{
<#
    .SYNOPSIS
      Reshapes the extent of an extendible HDF5 dataset
    .PARAMETER Path
      The path to an HDF5 dataset
    .PARAMETER Dimensions 
      The new extent of the HDF5 dataset
    .LINK
      New-H5Dataset
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The path to an extendible HDF5 dataset')]
        [ValidateNotNull()]
        [string]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=2,
                   HelpMessage='The new extent of the HDF5 dataset')]
        [ValidateNotNull()]
        [uint64[]]
        $Dimensions
    )

    if (!(Test-Path $Path -Resolvable))
    {
        Write-Error "`nDataset '$Path' not found!!!"
        return
    }

    Set-Item $Path $Dimensions
}
