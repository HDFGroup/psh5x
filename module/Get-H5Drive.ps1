
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
    .PARAMETER Name
      The name(s) of the H5Drive(s) to be removed.
    .PARAMETER Scope 
      The scope in which the H5Drive is defined. See about_Scopes.
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
                   HelpMessage='The name(s) of the H5Drive(s).')]
        [string[]]
        $Name,
        [Parameter(Mandatory=$false,
                   HelpMessage='The scope of the H5Drives. See about_Scopes.')]
        [int]
        $Scope = 2
    )

    if ($Name)
    {
        Write-Output(Get-PSDrive -PSProvider HDF5 -Name $Name -Scope $Scope)
    }
    else
    {
        Write-Output(Get-PSDrive -PSProvider HDF5 -Scope $Scope)
    }
}
