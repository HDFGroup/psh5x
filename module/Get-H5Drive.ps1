
Function Get-H5Drive
{
<#
    .SYNOPSIS
      Gets the Windows PowerShell drives backed by HDF5 files in the current
      session.
    .DESCRIPTION
      The Get-H5Drive function gets the Windows PowerShell drives backed by
      HDF5 files in the current session. You can a particular drive or all
      drives in the console.
    .EXAMPLE
      Get-H5Drive
    .EXAMPLE
      Get-H5Drive -Name h5
    .LINK
      Get-PSDrive
 #>
    param
    (
        [Parameter(Mandatory=$false,
                   ValueFromPipeline=$false,
                   HelpMessage='The name of the H5Drive.')]
        [string[]]
        $Name,
        [Parameter(Mandatory=$false,
                   ValueFromPipeline=$false,
                   HelpMessage='The scope of the H5Drives. See about_Scopes.')]
        [int]
        $Scope = 2
    )

    if ($Name)
    {
        Get-PSDrive -PSProvider HDF5 -Name $Name -Scope $Scope
    }
    else
    {
        Get-PSDrive -PSProvider HDF5 -Scope $Scope
    }
}
