
Function Remove-H5Drive(
                   [string] $name=($paramMissing=$true)
                   )
{
<#
    .SYNOPSIS
      Remove an HDF5 Drive 
    .DESCRIPTION
      The Remove-H5Drive function removes an PSDrive backed by an
      HDF5 file. It returns an error if the drive name is invalid.
    .EXAMPLE
      Remove-H5Drive -name h5
 #>
    If($local:paramMissing)
    {
        throw "USAGE: Remove-H5Drive -name <drive name>"
    }
    "`nAttempting to remove HDF5 drive $name ..."
    $count = 0
    Get-PSDrive -PSProvider HDF5 -Scope 2 | ?{$_.Name -eq $name} | %{$count++}
    If ($count -eq 1)
    {
        Remove-PSDrive -Name $name -Scope 2
        "`nSuccess: HDF5 drive $name removed."
    }
    Else
    {
        "`nError: HDF5 drive $name not found."
    }
}
