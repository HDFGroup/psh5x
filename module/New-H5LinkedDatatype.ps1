
Function New-H5LinkedDatatype
{
<#
    .SYNOPSIS
      Create a new HDF5 linked datatype (named datatype) 
    .DESCRIPTION
      The New-H5Group function creates one or more new HDF5 groups.
      Use the -Force switch to automatically create missing intermediate
      groups. Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .PARAMETER Path
     The path of the new HDF5 linked datatype.
   .PARAMETER Definition 
     The definition the new HDF5 datatype. The type can be specified as
     1) A pre-defined HDF5 datatype (string)
     2) An HDF5 datatype definition (hashtable)
     3) The HDF5 path name of a linked HDF5 datatype
   .PARAMETER Force
     Force the creation of intermediates.
   .EXAMPLE
     New-H5Datatype h5:\type1 H5T_NATIVE_INT
   .EXAMPLE
     $t = @{Class = 'String'; CharSet = 'Ascii'; Length = 17; StrPad = 'Nullterm' }

     New-H5Datatype /string16 $t 
   .LINK
     New-Item
   .NOTES
     Forward- (/) and backslash (\) seprators are supported in path names.
     The must not be used as part of link names.
     See URL for the HDF5 datatype hastable definition syntax.
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   Position=0,
                   HelpMessage='The path of the new HDF5 linked datatype.')]
        [string]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The definition the new HDF5 datatype.')]
        [object]
        $Definition,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    if ((Test-Path $Path))
    {
        Write-Error "`nThe path name '$Path' is in use."
        return
    }

    if (!(($Definition -is [string]) -or ($Definition -is [hashtable])))
    {
        Write-Error "`nInvalid type specification '$Definition' found."
        return
    }

    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 linked datatype'))
    { 
        try
        {
            if ($Force)
            {
                Write-Output(
                    New-Item $Path -ItemType Datatype -Definition $Definition `
                             -Force)
                Write-Host "`nSuccess: HDF5 linked datatype '$Path' created."
            }
            else
            {
                Write-Output(
                    New-Item $Path -ItemType Datatype -Definition $Definition)
                Write-Host "`nSuccess: HDF5 linked datatype '$Path' created."
            }
        }
        catch {
            Write-Debug($_|Out-String)
            Write-Error "`nUnable to create HDF5 linked datatype '$Path'. Intermediates missing? Is your current location an H5Drive?"
        }
    }
}
