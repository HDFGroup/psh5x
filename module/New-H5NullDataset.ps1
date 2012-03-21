
Function New-H5NullDataset
{
<#
    .SYNOPSIS
      Creates a new HDF5 null dataset (empty)
    .DESCRIPTION
      The New-H5NullDataset function creates a new HDF5 null dataset.
      Use the -Force switch to automatically create missing intermediate
      groups. Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
    .PARAMETER Path
      The path of the new HDF5 null dataset.
    .PARAMETER Type
      The element type of the HDF5 Dataset. The type can be specified as
      1) A pre-defined HDF5 datatype
      2) An HDF5 datatype definition (JSON)
      3) The HDF5 path name of a linked HDF5 datatype
    .PARAMETER Compact 
      Create an HDF5 dataset with compact layout.
    .PARAMETER Force
      Force the creation of intermediates.
    .EXAMPLE
    .LINK
      New-Item
    .NOTES
      Although an HDF5 null dataset cannot contain any data elements, an
      element type must be specified.

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
        [string]
        $Type,
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

    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 Scalar Dataset'))
    { 
        $param = @('-Path', $Path, '-ItemType', 'Dataset', '-ElementType', "'$Type'", '-Null')

        if ($Compact) {
            $param += '-Compact'
        }
        if ($Force) {
            $param += '-Force'
        }
        
        Write-Output(Invoke-Expression "New-Item $param")

        if (Test-Path $Path -Resolvable) {
            Write-Host "`nSuccess: HDF5 scalar dataset '$Path' created."
        }
    }
}
