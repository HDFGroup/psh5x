
Function Set-H5DatasetValue
{
<#
    .SYNOPSIS
      Sets the value of an HDF5 dataset
    .PARAMETER Path
      The path to an HDF5 dataset
    .PARAMETER Value
      The value of the HDF5 dataset
    .LINK
      Set-Content
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The path to an HDF5 dataset')]
        [ValidateNotNull()]
        [string]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=2,
                   HelpMessage='The value of the HDF5 dataset')]
        [ValidateNotNull()]
        [object]
        $Value
    )

    if (!(Test-Path $Path -Resolvable))
    {
        Write-Error "`nDataset '$Path' not found!!!"
        return
    }

    Set-Content $Path $Value
}
