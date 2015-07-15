
Function Add-H5Packets
{
<#
    .SYNOPSIS
      Adds packets to one or more HDF5 packet tables
    .PARAMETER Path
      The path name(s) to HDF5 datasets
    .PARAMETER Value
      The packets to be added to the HDF5 packet table(s)
    .PARAMETER PassThru
      Return an object representing the HDF5 packets adeded.
    .LINK
      Set-Content
    .LINK
      http://www.hdfgroup.org/HDF5/doc/HL/RM_H5PT.html#H5PTappend
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
                   HelpMessage='Write packets to pipeline?')]
        [switch]
        $PassThru
    )

    $cmd = 'Add-Content -Path $Path -Value $Value'
    
    if ($PassThru) {
        $cmd += ' -PassThru'
    }
    
    if ($PSCmdlet.ShouldProcess($Path, 'Adding HDF5 packets'))
    {
        Invoke-Expression $cmd
    }
}
