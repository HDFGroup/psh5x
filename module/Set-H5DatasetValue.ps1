
Function Set-H5DatasetValue
{
<#
    .SYNOPSIS
      Sets the value of an HDF5 dataset
    .PARAMETER Path
      The path to an HDF5 dataset
    .PARAMETER Value
      The value of the HDF5 dataset
    .PARAMETER Start
      The (multidimensional) offset where the hyperslab selection begins
    .PARAMETER Stride
      The (multidimensional) offset between consecutive blocks in the
      hyperslab selection
    .PARAMETER Count
      The (mulitdimensional) block count of the hyperslab selection
    .PARAMETER Block
      The (mulitdimensional) block size of the hyperslab selection
    .LINK
      Set-Content
      http://www.hdfgroup.org/HDF5/doc/UG/12_Dataspaces.html#DTransfer
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
        $Value,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=3,
                   HelpMessage='The start of the hyperslab selection')]
        [ValidateNotNull()]
        [uint64[]]
        $Start,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=4,
                   HelpMessage='The stride of the hyperslab selection')]
        [ValidateNotNull()]
        [uint64[]]
        $Stride,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=5,
                   HelpMessage='The block count of the hyperslab selection')]
        [ValidateNotNull()]
        [uint64[]]
        $Count,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=6,
                   HelpMessage='The block size of the hyperslab selection')]
        [ValidateNotNull()]
        [uint64[]]
        $Block
    )

    if (!(Test-Path $Path -Resolvable))
    {
        Write-Error "`nDataset '$Path' not found!"
        return
    }

    if (!($Start -or $Stride -or $Count -or $Block))
    {
        Set-Content $Path $Value
    }
    else
    {
        $cmd = 'Set-Content $Path $Value'
        if ($Start) {
            $cmd += ' -Start $Start'
        }
        if ($Stride) {
            $cmd += ' -Stride $Stride'
        }
        if ($Count) {
            $cmd += ' -Count $Count'
        }
        if ($Block) {
            $cmd += ' -Block $Block'
        }

        Invoke-Expression $cmd
    }
}
