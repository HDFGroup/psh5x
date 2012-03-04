
Function Get-H5DatasetValue
{
<#
    .SYNOPSIS
      Gets the value of an HDF5 dataset
    .PARAMETER Path
      The path to an HDF5 dataset
    .LINK
      Get-Content
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The path to an HDF5 dataset')]
        [ValidateNotNull()]
        [string]
        $Path
    )

    if (!(Test-Path $Path -Resolvable))
    {
        Write-Error "`nDataset '$Path' not found!!!"
        return
    }

    Get-Content $Path -ReadCount 0
}
