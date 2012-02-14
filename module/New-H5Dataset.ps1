
Function New-H5Dataset
{
<#
    .SYNOPSIS
      Creates a new HDF5 dataset 
    .DESCRIPTION
      The New-H5Dataset function creates a new HDF5 dataset.
      Use the -Force switch to automatically create missing intermediate
      groups. Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .PARAMETER Path
     The path of the new HDF5 Dataset.
   .PARAMETER Type
     The element type of the HDF5 Dataset. The type can be specified as
     1) A pre-defined HDF5 datatype (string)
     2) An HDF5 datatype definition (hashtable)
     3) The HDF5 path name of a linked HDF5 datatype
   .PARAMETER Nulll
     Create an HDF5 dataset with a null dataspace (empty set).
   .PARAMETER Scalar
     Create an HDF5 dataset with a scalar dataspace (singleton).
   .PARAMETER Dimensions
     Create an HDF5 dataset with a simple dataspace.
   .PARAMETER MaxDimensions
     Create an HDF5 dataset with a simple extendible dataspace.
   .PARAMETER Chunked 
     Create an HDF5 dataset with chunked layout.
   .PARAMETER Gzip 
     Create an HDF5 dataset with chunked layout and gzip compression enabled.
   .PARAMETER Compact 
     Create an HDF5 dataset with compact layout.
   .PARAMETER Force
     Force the creation of intermediates.
   .EXAMPLE
   .LINK
     New-Item
   .NOTES
     Forward- (/) and backslash (\) seprators are supported in path names.
     The must not be used as part of link names.
 #>
    [CmdletBinding(SupportsShouldProcess=$true,
                   DefaultParametersetName='Simple')]
    param
    (
        [Parameter(Mandatory=$true,
                   Position=0,
                   HelpMessage='The path to the new HDF5 dataset.')]
        [string]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The element type of the new HDF5 dataset.')]
        [ValidateNotNull()]
        [object]
        $Type,
        [Parameter(Mandatory=$true,
                   Position=2,
                   ParameterSetName='Null',
                   HelpMessage='Null dataspace?')]
        [switch]
        $Nulll,
        [Parameter(Mandatory=$true,
                   Position=2,
                   ParameterSetName='Scalar',
                   HelpMessage='Scalar dataspace?')]
        [switch]
        $Scalar,
        [Parameter(Mandatory=$true,
                   Position=2,
                   ParameterSetName='Simple',
                   HelpMessage='Dimensions of the simple dataspace')]
        [ValidateCount(1,32)]
        [long[]]
        $Dimensions,
        [Parameter(Mandatory=$false,
                   Position=3,
                   ParameterSetName='Simple',
                   HelpMessage='Max. dimensions of the simple dataspace')]
        [ValidateCount(1,32)]
        [long[]]
        $MaxDimensions,
        [Parameter(Mandatory=$false,
                   Position=4,
                   ParameterSetName='Simple',
                   HelpMessage='Chunk dimensions')]
        [ValidateCount(1,32)]
        [long[]]
        $Chunked,
        [Parameter(Mandatory=$false,
                   Position=5,
                   ParameterSetName='Simple',
                   HelpMessage='Gzip level (0-9)')]
        [ValidateRange(0,9)]
        [int]
        $Gzip,
        [Parameter(Mandatory=$false,
                   HelpMessage='Compact layout?')]
        [switch]
        $Compact,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    if ((Test-Path $Path))
    {
        Write-Error "`nThe path name '$Path' is in use."
        return
    }
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

    $typedef = $Type.GetType().FullName
    if (!(($typedef -eq 'System.String') `
          -or ($typedef -eq 'System.Collections.Hashtable')))
    {
        Write-Error "`nInvalid type specification '$Type' found."
        return
    }

    if ($Nulll)
    {
        Write-Error "`nSupport for null datasets unimplemented!!!"
        return
    }
    elseif ($Scalar)
    {
        Write-Error "`nSupport for scalar datasets unimplemented!!!"
        return
    }
    else #simple dataspace
    {
        for ($i = 0; $i -lt $Dimensions.Length; $i++)
        {
            if (!($Dimensions[$i] -gt 0))
            {
                Write-Error "`nDimensions must be positive."
                return
            }
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
        else {
            $MaxDimensions = $Dimensions
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
                if (!($Chunked[$i] -gt 0))
                {
                    Write-Error "`nChunk dimensions must be positive."
                    return
                }
                if (($MaxDimensions[$i] -gt 0) -and
                    !($MaxDimensions[$i] -ge $Chunked[$i]))
                {
                    Write-Error "`nFor limited maximum dimensions the chunk dimension must not exceed the maximum dimension."
                    return
                }
            }
        }
    }

    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 Dataset'))
    { 
        if ($Chunked)
        {
            if ($Gzip)
            {
                if ($Force) {
                    Write-Output(
                        New-Item $Path -ItemType Dataset -ElementType $type `
                                 -Dimensions $Dimensions -MaxDimensions `
                                 $MaxDimensions -Chunked $Chunked -Gzip $Gzip `
                                 -Force)
                }
                else {
                    Write-Output(
                        New-Item $Path -ItemType Dataset -ElementType $type `
                                 -Dimensions $Dimensions -MaxDimensions `
                                 $MaxDimensions -Chunked $Chunked -Gzip $Gzip)
                }
            }
            else
            {
                if ($Force) {
                    Write-Output(
                        New-Item $Path -ItemType Dataset -ElementType $type `
                                 -Dimensions $Dimensions -MaxDimensions `
                                 $MaxDimensions -Chunked $Chunked -Force)
                }
                else {
                    Write-Output(
                        New-Item $Path -ItemType Dataset -ElementType $type `
                                 -Dimensions $Dimensions -MaxDimensions `
                                 $MaxDimensions -Chunked $Chunked)
                }
            }
        }
        elseif ($Compact)
        {
            if ($Null)
            {
            }
            elseif ($Scalar)
            {
            }
            else
            {
                if ($Force) {
                    Write-Output( `
                        New-Item $Path -ItemType Dataset -ElementType $type `
                                 -Dimensions $Dimensions -Compact -Force)
                }
                else {
                    Write-Output( `
                        New-Item $Path -ItemType Dataset -ElementType $type `
                                 -Dimensions $Dimensions -Compact)
                }
            }
        }
        else #contiguous layout
        {
            if ($Null)
            {
            }
            elseif ($Scalar)
            {
            }
            else
            {
                if ($Force) {
                    Write-Output( `
                        New-Item $Path -ItemType Dataset -ElementType $type `
                                 -Dimensions $Dimensions -Force)
                }
                else {
                    Write-Output( `
                        New-Item $Path -ItemType Dataset -ElementType $type `
                                 -Dimensions $Dimensions)
                }
            }
        }
    }
}
