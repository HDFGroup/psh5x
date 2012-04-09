
Function New-H5Attribute
{
<#
    .SYNOPSIS
      Creates new HDF5 Attributes 
    .DESCRIPTION
      The New-H5Attribute function creates a new HDF5 attribute
      of one or more HDF5 objects (group, dataset, linked datatype).
      
      If only a path name, an attribute name, and a value are specified,
      the function attempts to create a variable-length string,
      scalar HDF5 attribute.
   .PARAMETER Path
     The path to the HDF5 object(s) (group, dataset, linked datatype) to decorate.
   .PARAMETER Name
     The name of the HDF5 attribute
   .PARAMETER Scalar 
     The HDF5 attribute value of a scalar attribute
   .PARAMETER Simple 
     Create an HDF5 attribute with a simple dataspace.
   .PARAMETER Nulll
     Create an HDF5 attribute with a null dataspace (empty set).
   .PARAMETER Type
     The element type of the HDF5 attribute. The type can be specified as
     1) A pre-defined HDF5 datatype (string)
     2) An HDF5 datatype definition (hashtable)
     3) The HDF5 path name of a linked HDF5 datatype
   .PARAMETER Value 
     The HDF5 attribute value (simple attributes only)
   .EXAMPLE
     New-H5Attribute /groupA stringAttr 'bar one'
   .EXAMPLE
     New-H5Attribute -Path h5:\groupA -Name intAttr -Scalar 4711 -Type int
   .LINK
     New-ItemProperty
   .NOTES
     Forward- (/) and backslash (\) seprators are supported in path names.
     The must not be used as part of link names.
 #>
    [CmdletBinding(SupportsShouldProcess=$true,
                   DefaultParametersetName='Scalar')]
    param
    (
        [Parameter(Mandatory=$true,
                   Position=0,
                   HelpMessage='The path to the HDF5 object to decorate')]
        [ValidateNotNull()]
        [string[]]
        $Path,
        [Parameter(Mandatory=$true,
                   ValueFromPipelineByPropertyName=$true,
                   Position=1,
                   HelpMessage='The name of the new HDF5 attribute')]
        [ValidateNotNull()]
        [string]
        $Name,
        [Parameter(Mandatory=$true,
                   Position=2,
                   ParameterSetName='Simple',
                   HelpMessage='Dimensions of the simple dataspace')]
        [ValidateNotNull()]
        [ValidateCount(1,32)]
        [uint64[]]
        $Simple,
        [Parameter(Mandatory=$true,
                   Position=2,
                   ParameterSetName='Null',
                   HelpMessage='Null dataspace?')]
        [switch]
        $Nulll,
        [Parameter(Mandatory=$false,
                   Position=2,
                   ParameterSetName='Scalar',
                   HelpMessage='Scalar attribute value')]
        [ValidateNotNull()]
        [object]
        $Scalar,
        [Parameter(Mandatory=$false,
                   Position=3,
                   HelpMessage='The element type of the new HDF5 attribute')]
        [ValidateNotNull()]
        [string]
        $Type='string',
        [Parameter(Mandatory=$false,
                   ParameterSetName='Simple',
                   ValueFromPipelineByPropertyName=$true,
                   Position=4,
                   HelpMessage='Non-scalar attribute value')]
        [ValidateNotNull()]
        [object]
        $Value
    )

    if ($Nulll -and $Value) {
        Write-Host "`nWarning: -Value has no effect for attributes with null dataspaces."
    }
    
    $cmd = 'New-ItemProperty $Path -Name $Name -ElementType $Type'
    
    if ($Nulll) {
        $cmd += ' -Null'
    }
    elseif ($Simple) # simple attribute
    {
        $cmd += ' -Simple $Simple'
            
        if ($Value) {
            $cmd += '-Value $Value'
        }
    }
    else # scalar attribute
    {
        if ($Scalar) {
            $cmd += ' -Value $Scalar'
        }
    }

    if ($PSCmdlet.ShouldProcess($Path, "New HDF5 Attribute '$Name'"))
    {     
        Invoke-Expression $cmd
    }
}
