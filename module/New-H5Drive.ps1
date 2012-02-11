
Function New-H5Drive
{
<#
    .SYNOPSIS
      Create a new H5Drive (=PSDrive backed by an HDF5 file)
    .DESCRIPTION
      The New-H5Drive function creates a new PSDrive backed by the specified
      HDF5 file. It returns an error if the drive name is in use, the file
      doesn't exist, or the user has insufficient permissions.
    .EXAMPLE
      New-H5Drive -Name h5 -File C:\tmp\foo.h5
    .EXAMPLE
      New-H5Drive -Name h5 -File C:\tmp\foo.h5 -rw
    .LINK
      New-PSDrive
      about_Scopes
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipeline=$false,
                   HelpMessage='The name of the H5Drive.')]
        [string]
        $Name=($paramMissing=$true),
        [Parameter(Mandatory=$true,
                   ValueFromPipeline=$false,
                   HelpMessage='The name of an HDF5 file.')]
        [string]
        $File,
        [Parameter(Mandatory=$false,
                   ValueFromPipeline=$false,
                   HelpMessage='Make the H5Drive writeable?')]
        [switch]
        $RW,
        [Parameter(Mandatory=$false,
                   ValueFromPipeline=$false,
                   HelpMessage='The root of the H5Drive.')]
        [string] $Root,
        [Parameter(Mandatory=$false,
                   ValueFromPipeline=$false,
                   HelpMessage='The scope of the H5Drive. See about_Scopes.')]
        [int]    $Scope=2
    )

    if (!(Test-H5File $File))
    {
        throw "Error: File '$File' is not an HDF5 file."
    }

    if (!$Root)
    {
        $Root = "$($Name):\"
    }

    if ($RW)
    {
        "`nAttempting to create new WRITEABLE H5Drive '$Name' with file '$File', and root '$Root' ..."
        $status = (New-PSDrive -Name $Name -PSProvider HDF5 -Path $File -Root $Root -Mode RW -Scope $Scope)
    }
    else
    {
        "`nAttempting to create new READ-ONLY HDF5 drive '$Name' with file '$File', and root '$Root' ..."
        $status = (New-PSDrive -Name $Name -PSProvider HDF5 -Path $File -Root $Root -Scope $Scope)
    }

    $count = 0
    Get-PSDrive -PSProvider HDF5 -Scope $Scope | ?{$_.Name -eq $name} | %{ $count++ }
    if ($count -eq 1)
    {
        "`nSuccess: HDF5 drive '$Name' created."
        $status
    }
    else
    {
        "`nError: Creation of HDF5 drive '$Name' failed."
    }
}
