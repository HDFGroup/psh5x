
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
    .LINK
      about_Scopes
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The name of the H5Drive.')]
        [string[]]
        $Name,
        [Parameter(Mandatory=$false,
                   ValueFromPipelineByPropertyName=$true,
                   Position=1,
                   HelpMessage='The scope of the H5Drive. See about_Scopes.')]
        [string]
        $Scope = 2
    )
    
    $cmd = 'Remove-PSDrive -Name $Name'
    
    if ($Scope) {
        $cmd += ' -Scope $Scope'
    }
    
    if ($PSCmdlet.ShouldProcess($Name, 'Remove HDF5 Drive'))
    {
        Invoke-Expression $cmd
    }
}
