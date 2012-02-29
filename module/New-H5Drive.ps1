
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
                   HelpMessage='The name of the H5Drive.')]
        [string]
        $Name,
        [Parameter(Mandatory=$true,
                   HelpMessage='The name of an HDF5 file.')]
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
        [int]
        $Scope=2
    )

    if (!(Test-H5File (Resolve-Path $File)))
    {
        Write-Error "`nFile '$File' is not an HDF5 file."
        return
    }
    else {
        $File = Resolve-Path $File
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

    if ($PSCmdlet.ShouldProcess($File, "New HDF5 Drive '$Name'"))
    { 
        try
        {
            if ($RW)
            {
                Write-Output(
                    New-PSDrive -Name $Name -PSProvider HDF5 -Path $File `
                                -Root $Root -Mode RW -Scope $Scope)
                $count = 0
                Get-PSDrive | ?{$_.Name -eq $Name} | %{$count++}
                if ($count -eq 1) {
                    Write-Host "`nSuccess: WRITEABLE H5Drive '$Name' created."
                }
            }
            else
            {
                Write-Output(
                    New-PSDrive -Name $Name -PSProvider HDF5 -Path $File `
                                -Root $Root -Scope $Scope)
                $count = 0
                Get-PSDrive | ?{$_.Name -eq $Name} | %{$count++}
                if ($count -eq 1) {
                    Write-Host "`nSuccess: READ-ONLY H5Drive '$Name' created."
                }
            }
        }
        catch {
            Write-Debug ($_| Out-String)
            Write-Error "`nCreation of HDF5 drive '$Name' failed."
        }
    }
}
