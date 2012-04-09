
Function New-H5LinkedDatatype
{
<#
    .SYNOPSIS
      Create a new HDF5 linked datatype ("named" datatype) 
    .DESCRIPTION
      The New-H5LinkedDatatype function creates one or more new
      HDF5 linked datatypes.
    .PARAMETER Path
      The path(s) of the new HDF5 linked datatype.
    .PARAMETER Definition 
      The definition the new HDF5 datatype. The type can be specified as
      1) A predefined HDF5 datatype (string)
      2) An HDF5 datatype definition (JSON)
      3) An HDF5 path name of a linked datatype (string) 
    .PARAMETER Force
      Force the automatic creation of intermediate HDF5 groups.
    .EXAMPLE
      New-H5Datatype h5:\type1 H5T_NATIVE_INT
    .EXAMPLE
      $t = '{"Class":"String", "CharSet":"Ascii", "Length":17, "StrPad":"Nullterm" }'
    .LINK
      New-Item
    .LINK
      http://www.hdfgroup.org/HDF5/doc/RM/RM_H5T.html#Datatype-Commit2  
    .LINK
      about_H5Datatypes
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=0,
                   HelpMessage='The path of the new HDF5 linked datatype.')]
        [string[]]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The definition the new HDF5 datatype.')]
        [string]
        $Definition,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )
    
    $cmd = 'New-Item -Path $Path -ItemType Datatype -Definition $Definition'
    
    if ($Force) {
        $cmd += ' -Force'
    }

    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 linked datatype'))
    { 
        Invoke-Expression $cmd
    }
}
