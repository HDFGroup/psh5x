
Function Test-H5Attribute
{
<#
    .SYNOPSIS
      Tests if an HDF5 object has an HDF5 Attribute of a given name
    .DESCRIPTION
      The Test-H5Attribute function probes the existence of an HDF5 attribute
      of an HDF5 object (group, dataset, linked datatype).

      Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .PARAMETER Path
     The path to the HDF5 object (group, dataset, linked datatype)
   .PARAMETER Name
     The name of the HDF5 attribute
   .EXAMPLE
     Test-H5Attribute h5:\groupA foo

   .LINK
     Test-Path
   .NOTES
     Forward- (/) and backslash (\) seprators are supported in path names.
     The must not be used as part of link names.
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   Position=0,
                   HelpMessage='The path to the HDF5 object')]
        [ValidateNotNull()]
        [string]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The name of the HDF5 attribute')]
        [ValidateNotNull()]
        [string]
        $Name
    )

    if (!(Test-Path $Path))
    {
        Write-Error "`nThe path name '$Path' is invalid."
        return
    }

    Write-Output((Get-Item $Path).AttributeNames -Contains $Name)
}
