
Function New-H5Dataset
{
<#
    .SYNOPSIS
      Creates a new HDF5 simple dataset 
    .DESCRIPTION
      The New-H5Dataset function creates a new HDF5 simple dataset.
      Use the -Force switch to automatically create missing intermediate
      groups.
    .PARAMETER Path
      The path of the new HDF5 Dataset.
    .PARAMETER Type
      The element type of the HDF5 Dataset. The type can be specified as
      1) A pre-defined HDF5 datatype
      2) An HDF5 datatype definition (JSON)
      3) The HDF5 path name of a linked HDF5 datatype
    .PARAMETER Null
      Create an HDF5 dataset with a null dataspace (void).
    .PARAMETER Scalar
      Create an HDF5 dataset with a scalar dataspace (singleton).
    .PARAMETER Dimensions
      Create an HDF5 dataset with a simple dataspace of this extent.
    .PARAMETER MaxDimensions
      Create an HDF5 dataset with a simple extendible dataspace.
    .PARAMETER Chunked 
      Create an HDF5 dataset with chunked layout and this chunk size.
    .PARAMETER Gzip 
      Create an HDF5 dataset (with chunked layout) with gzip compression
      enabled.
    .PARAMETER Compact 
      Create an HDF5 dataset with compact layout.
    .PARAMETER FillValue 
      Create an HDF5 dataset with this fill value.
    .PARAMETER Force
      Force the automatic creation of intermediate HDF5 groups.
    .EXAMPLE
    .LINK
      New-Item
 #>
    [CmdletBinding(SupportsShouldProcess=$true,
                   DefaultParametersetName='Simple')]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The path to the new HDF5 dataset.')]
        [ValidateNotNull()]
        [string[]]
        $Path,
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=1,
                   HelpMessage='The element type of the new HDF5 dataset.')]
        [ValidateNotNull()]
        [string]
        $Type,
        [Parameter(Mandatory=$true,
                   ParameterSetName='Null',
                   Position=2,
                   HelpMessage='Create a dataset with a null dataspace?')]
        [switch]
        $Nulll,
        [Parameter(Mandatory=$true,
                   ParameterSetName='Scalar',
                   Position=2,
                   HelpMessage='Create a dataset with a scalar dataspace?')]
        [switch]
        $Scalar,
        [Parameter(Mandatory=$true,
                   ParameterSetName='Simple',
                   Position=2,
                   HelpMessage='Extent (dimensions) of the simple dataspace')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [uint64[]]
        $Dimensions,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Simple',
                   Position=3,
                   HelpMessage='Max. dimensions of the simple dataspace')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [long[]]
        $MaxDimensions,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Simple',
                   Position=4,
                   HelpMessage='Chunk dimensions')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [uint64[]]
        $Chunked,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Simple',
                   Position=5,
                   HelpMessage='Gzip level (0-9)')]
        [ValidateRange(0,9)]
        [int]
        $Gzip,
        [Parameter(Mandatory=$false,
                   HelpMessage='Compact layout?')]
        [switch]
        $Compact,
        [Parameter(Mandatory=$false,
                   HelpMessage='FillValue?')]
        [ValidateNotNull()]
        [object]
        $FillValue,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    if ($Chunked -and $Compact)
    {
        Write-Error "`nThe -Chunked and -Compact options are not compatible."
        return
    }
    if (!$Chunked -and $Gzip)
    {
        Write-Error "`nThe -Gzip option requires chunked layout. (-Chunked...)."
        return
    }
    if ($MaxDimensions)
    {
        if ($Dimensions.Length -ne $MaxDimensions.Length)
        {
            Write-Error "`nLength mismatch between dimension ($($Dimensions.Length)) and max. dimension ($($MaxDimensions.Length)) arrays"
            return
        }

        $equal = $true
        for ($i = 0; $i -lt $Dimensions.Length; $i++)
        {
            if (!(($MaxDimensions[$i] -lt 0) `
                  -or ($MaxDimensions[$i] -ge $Dimensions[$i])))
            {
                Write-Error "`nMax. dimensions must be either unlimited or greater or equal than the dimensions"
                return
            }
            if ($MaxDimensions[$i] -ne $Dimensions[$i]) {
                $equal = $false
            }
        }

        if (!($equal -or $Chunked))
        {
            Write-Error "`nExtendible datasets must be chunked. Use -Chunked ... !"
            return
        }
    }
    
    if ($Chunked)
    {
        if ($Dimensions.Length -ne $Chunked.Length)
        {
            Write-Error "`nLength mismatch between dimension ($($Dimensions.Length)) and chunk dimension ($($Chunked.Length)) arrays"
            return
        }

        for ($i = 0; $i -lt $Dimensions.Length; $i++)
        {
            if (($MaxDimensions[$i] -gt 0) -and
                !($MaxDimensions[$i] -ge $Chunked[$i]))
            {
                Write-Error "`nFor limited maximum dimensions the chunk dimension must not exceed the maximum dimension."
                return
            }
        }
    }
    
    $cmd = 'New-Item -Path $Path -ItemType Dataset -ElementType $type'
        
    if ($Force) {
        $cmd += ' -Force'
    }
    if ($FillValue) {
        $cmd += ' -Value $FillValue'
    }
    if ($Compact) {
        $cmd += ' -Compact'
    }
    if ($Nulll) {
        $cmd += ' -Null'
    }
    elseif ($Scalar) {
        $cmd += ' -Scalar'
    }
    else
    {
       $cmd += ' -Dimensions $Dimensions'
            
        if ($MaxDimensions)
        {
            $cmd += ' -MaxDimensions $MaxDimensions'
            
            if ($Chunked)
            {
                $cmd += ' -Chunked $Chunked'
                
                if ($Gzip) {
                    $cmd += ' -Gzip $Gzip'
                }
            }
        }
    }
        
    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 Dataset'))
    {     
        Invoke-Expression $cmd
    }
}
