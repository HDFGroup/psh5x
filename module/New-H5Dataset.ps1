
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
     The path of the new HDF5 group(s).
   .PARAMETER Force
     Force the creation of intermediates.
   .PARAMETER WhatIf
     What if?
   .EXAMPLE
     New-H5Group -Path h5:\group1
   .EXAMPLE
     New-H5Group -Path h5:\group2\group3 -Force
   .EXAMPLE
     New-H5Group -Path h5:/group1
   .EXAMPLE
     New-H5Group -Path h5:/group1/group2
   .LINK
     New-Item
   .NOTES
     Forward- (/) and backslash (\) seprators are supported in path names.
     The must not be used as part of link names.
 #>
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
        $Force,
        [Parameter(Mandatory=$false,
                   HelpMessage='What if?')]
        [switch]
        $WhatIf
    )

    if ((Test-Path $Path))
    {
        Write-Host "`nError: The path name '$Path' is in use."
        return
    }

    $typedef = $Type.GetType().FullName
    if (!(($typedef -eq 'System.String') `
          -or ($typedef -eq 'System.Collections.Hashtable')))
    {
        Write-Host "`nError: Invalid type specification '$Type' found."
        return
    }

    if ($Nulll)
    {
        #Write-Output(New-Item $Path -ItemType Dataset -ElementType $Type -Null)
        Write-Host "`nError: Support for null datasets unimplemented!!!"
        return
    }
    elseif ($Scalar)
    {
        #Write-Output(New-Item $Path -ItemType Dataset -ElementType $Type -Scalar)
        Write-Host "`nError: Support for scalar datasets unimplemented!!!"
        return
    }
    else # Simple
    {
        if ($MaxDimensions)
        {
            if ($Dimensions.Length -ne $MaxDimensions.Length)
            {
                Write-Host "`nError: Length mismatch between dimension ($($Dimensions.Length)) and max. dimension ($($MaxDimensions.Length)) arrays"
                return
            }
        }

        if ($Chunked)
        {
            if ($Dimensions.Length -ne $Chunked.Length)
            {
                Write-Host "`nError: Length mismatch between dimension ($($Dimensions.Length)) and chunk dimension ($($Chunked.Length)) arrays"
                return
            }
        }
    }
}
