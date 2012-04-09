
Function New-H5Drive
{
<#
    .SYNOPSIS
      Creates a new H5Drive (=PSDrive backed by an HDF5 file)
    .DESCRIPTION
      The New-H5Drive function creates a new PSDrive backed by the specified
      HDF5 file. It returns an error if the drive name is in use, the file
      doesn't exist, or the user has insufficient permissions.
    .PARAMETER Name
      The name of the H5Drive.
    .PARAMETER File 
      The name of an HDF5 file.
    .PARAMETER RW
      Make the H5Drive writeable?
    .PARAMETER Root
      The root of the H5Drive, the path name of a group in the HDF5 file.
      The HDF5 root group is the default.
    .PARAMETER Scope
      The scope in which the drive is to be created. See about_Scopes.
    .PARAMETER Force
      If the HDF5 does not exist, force the creation of a new HDF5 file.
    .EXAMPLE
      New-H5Drive -Name h5 -File C:\tmp\foo.h5
    .EXAMPLE
      New-H5Drive -Name h5 -File C:\tmp\foo.h5 -RW
      Create a writeable H5Drive.
    .LINK
      New-PSDrive
      about_Scopes
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=1,
                   HelpMessage='The name of the H5Drive.')]
        [ValidateNotNull()]
        [string]
        $Name,
        [Parameter(Mandatory=$true,
                   Position=2,
                   HelpMessage='The name of an HDF5 file.')]
        [ValidateNotNull()]
        [string]
        $File,
        [Parameter(Mandatory=$false,
                   HelpMessage='Make the H5Drive writeable?')]
        [switch]
        $RW,
        [Parameter(Mandatory=$false,
                   HelpMessage='The root of the H5Drive.')]
        [string]
        $Root,
        [Parameter(Mandatory=$false,
                   HelpMessage='The scope of the H5Drive. See about_Scopes.')]
        [string]
        $Scope='2',
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of a new file?')]
        [switch]
        $Force
    )

    if (!($Force -or (Test-H5File (Resolve-Path $File))))
    {
        Write-Error "`nFile '$File' is not an HDF5 file."
        return
    }
    
    $count = 0
    Get-PSDrive | ?{$_.Name -eq $Name} | %{$count++}
    if ($count -eq 1)
    {
        Write-Error "`nDrive name '$Name' is in use."
        return
    }
    
    if (!$Root)
    {
        $Root = "$($Name):\"
    }
    
    $cmd = 'New-PSDrive -Name $Name -PSProvider HDF5 -Path $File -Root $Root'
        
    if ($RW) {
        $cmd += ' -Mode RW'
    }
        
    if ($Scope) {
        $cmd += ' -Scope $Scope'
    }
        
    if ($Force) {
        $cmd += ' -Force'
    }
        
    if ($PSCmdlet.ShouldProcess($File, "New HDF5 Drive '$Name'"))
    {
       
        Invoke-Expression $cmd
    }
}
