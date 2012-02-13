
Function Remove-H5Drive
{
<#
    .SYNOPSIS
      Remove an H5Drive (=PSDrive backed by an HDF5 file)
    .DESCRIPTION
      The Remove-H5Drive function removes a PSDrive backed by an
      HDF5 file. It returns an error if the drive name is invalid
      or the drive is busy, e.g., your current location is on the drive.
    .PARAMETER Name
      The name(s) of the H5Drive(s) to be removed.
    .PARAMETER Scope 
      The scope in which the H5Drive is defined. See about_Scopes.
    .EXAMPLE
      Remove-H5Drive -Name h5
    .EXAMPLE
      Remove-H5Drive -Name h5,h51
    .LINK
      Remove-PSDrive
      about_Scopes
 #>
    param
    (
        [Parameter(Mandatory=$true,
                   HelpMessage='The name of the H5Drive.')]
        [string[]]
        $Name,
        [Parameter(Mandatory=$false,
                   HelpMessage='The scope of the H5Drive. See about_Scopes.')]
        [int]
        $Scope = 2
    )

    try
    {
        Remove-PSDrive -Name $Name -Scope $Scope 
    }
    catch {
        Write-Debug($_|Out-String)
    }
}
