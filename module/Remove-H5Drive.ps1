
Function Remove-H5Drive
{
<#
    .SYNOPSIS
      Remove an H5Drive (=PSDrive backed by an HDF5 file)
    .DESCRIPTION
      The Remove-H5Drive function removes a PSDrive backed by an
      HDF5 file. It returns an error if the drive name is invalid
      or the drive is busy, e.g., your current location is on the drive.
    .EXAMPLE
      Remove-H5Drive -Name h5
    .LINK
      Remove-PSDrive
      about_Scopes
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipeline=$false,
                   HelpMessage='The name of the H5Drive.')]
        [string]
        $Name,
        [Parameter(Mandatory=$false,
                   ValueFromPipeline=$false,
                   HelpMessage='The scope of the H5Drive. See about_Scopes.')]
        [int]
        $Scope = 2
    )

    "`nAttempting to remove HDF5 drive '$Name' ..."

    $count = 0
    Get-PSDrive -PSProvider HDF5 -Scope $Scope | ?{$_.Name -eq $Name} | %{$count++}

    if ($count -eq 1)
    {
        Remove-PSDrive -Name $Name -Scope $Scope 
        "`nSuccess: HDF5 drive '$Name' removed."
    }
    else
    {
        "`nError: HDF5 drive '$Name' not found."
    }
}
