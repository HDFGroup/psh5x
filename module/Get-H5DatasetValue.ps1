
Function Get-H5DatasetValue
{
<#
    .SYNOPSIS
      Gets the value of an HDF5 dataset
    .PARAMETER Path
      The path to an HDF5 dataset
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
      Get-Content
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
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=2,
                   HelpMessage='The start of the hyperslab selection')]
        [ValidateNotNull()]
        [uint64[]]
        $Start,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=3,
                   HelpMessage='The stride of the hyperslab selection')]
        [ValidateNotNull()]
        [uint64[]]
        $Stride,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=4,
                   HelpMessage='The block count of the hyperslab selection')]
        [ValidateNotNull()]
        [uint64[]]
        $Count,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=5,
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
        Get-Content $Path -ReadCount 0
    }
    else
    {
        $cmd = 'Get-Content $Path -ReadCount 0'
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
