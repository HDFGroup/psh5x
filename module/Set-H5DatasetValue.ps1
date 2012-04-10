
Function Set-H5DatasetValue
{
<#
    .SYNOPSIS
      Sets the value of an HDF5 dataset (scalar and simple)
    .PARAMETER Path
      The path name(s) to HDF5 datasets
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
    .PARAMETER PassThru
      Return an object representing the update value.
    .LINK
      Set-Content
    .LINK
      http://www.hdfgroup.org/HDF5/doc/RM/RM_H5D.html#Dataset-Write
    .LINK
      http://www.hdfgroup.org/HDF5/doc/UG/12_Dataspaces.html#DTransfer
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The path to an HDF5 dataset')]
        [ValidateNotNull()]
        [string[]]
        $Path,
        [Parameter(Mandatory=$true,
                   ValueFromPipeline=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=1,
                   HelpMessage='The value of the HDF5 dataset')]
        [ValidateNotNull()]
        [object[]]
        $Value,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=2,
                   HelpMessage='The start of the hyperslab selection')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [uint64[]]
        $Start,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=3,
                   HelpMessage='The stride of the hyperslab selection')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [uint64[]]
        $Stride,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=4,
                   HelpMessage='The block count of the hyperslab selection')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [uint64[]]
        $Count,
        [Parameter(Mandatory=$false,
                   ParameterSetName='Hyperslab',
                   Position=5,
                   HelpMessage='The block size of the hyperslab selection')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [uint64[]]
        $Block,
        [Parameter(Mandatory=$false,
                   HelpMessage='Write objects to pipeline?')]
        [switch]
        $PassThru
    )

    $cmd = 'Set-Content -Path $Path -Value $Value'
    
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
    
    if ($PassThru) {
        $cmd += ' -PassThru'
    }
    
    if ($PSCmdlet.ShouldProcess($Path, 'Setting HDF5 dataset value'))
    {
        Invoke-Expression $cmd
    }
}
