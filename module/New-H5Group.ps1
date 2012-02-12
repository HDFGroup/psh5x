
Function New-H5Group
{
<#
    .SYNOPSIS
      Create a new HDF5 group 
    .DESCRIPTION
      The New-H5Group function creates one or more new HDF5 groups.
      Use the -Force switch to automatically create missing intermediate
      groups. Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .EXAMPLE
      New-H5Group -Path h5:\group1
    .EXAMPLE
      New-H5Group -Path h5:\group2\group3 -Force
    .LINK
      New-Item
    .NOTES
      Forward- (/) and backslash (\) seprators are supported in path names.
      The must not be used as part of link names.
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipeline=$false,
                   HelpMessage='The path of the new HDF5 group(s).')]
        [string[]]
        $Path,
        [Parameter(Mandatory=$false,
                   ValueFromPipeline=$false,
                   HelpMessage='Force the creation of intermediates.')]
        [switch]
        $Force
    )

    if ($Force)
    {
        New-Item -Path $Path -ItemType Group -Force
    }
    else
    {
        New-Item -Path $Path -ItemType Group
    }
}
