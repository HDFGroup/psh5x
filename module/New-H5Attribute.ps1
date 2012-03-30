
Function New-H5Attribute
{
<#
    .SYNOPSIS
      Creates a new HDF5 Attribute 
    .DESCRIPTION
      The New-H5Attribute function creates a new HDF5 attribute
      of an HDF5 object (group, dataset, linked datatype).
      If only a path, name and value are specified, the function attempts
      to create a variable-length string, scalar HDF5 attribute.

      Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .PARAMETER Path
     The path to the HDF5 object (group, dataset, linked datatype) to decorate.
   .PARAMETER Name
     The name of the HDF5 attribute
   .PARAMETER Value 
     The HDF5 attribute value
   .PARAMETER Simple 
     Create an HDF5 attribute with a simple dataspace.
   .PARAMETER Nulll
     Create an HDF5 attribute with a null dataspace (empty set).
   .PARAMETER Type
     The element type of the HDF5 attribute. The type can be specified as
     1) A pre-defined HDF5 datatype (string)
     2) An HDF5 datatype definition (hashtable)
     3) The HDF5 path name of a linked HDF5 datatype
   .EXAMPLE
     New-H5Attribute h5:\groupA foo bar

     New-H5Attribute h5:\groupA intAttr 4711 -Type int
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
        [string]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=1,
                   HelpMessage='The name of the new HDF5 attribute')]
        [ValidateNotNull()]
        [string]
        $Name,
        [Parameter(Mandatory=$true,
                   Position=2,
                   ParameterSetName='Simple',
                   HelpMessage='Dimensions of the simple dataspace')]
        [ValidateCount(1,32)]
        [long[]]
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
                   Position=4,
                   HelpMessage='Non-scalar attribute value')]
        [ValidateNotNull()]
        [object]
        $Value
    )

    if (!(Test-Path $Path))
    {
        Write-Error "`nThe path name '$Path' is invalid."
        return
    }
    if (Test-H5Attribute $Path $Name)
    {
        Write-Error "`nThe object '$Path' already has a '$Name' attribute."
        return
    }

    if ($Nulll -and $Value) {
        Write-Host "`nWarning: -Value has no effect for attributes with null dataspaces."
    }

    $typedef = $Type.GetType().FullName
    if (!(($typedef -eq 'System.String') `
          -or ($typedef -eq 'System.Collections.Hashtable')))
    {
        Write-Error "`nInvalid type specification '$Type' found."
        return
    }

    if ($Simple) #simple dataspace
    {
        for ($i = 0; $i -lt $Simple.Length; $i++)
        {
            if (!($Simple[$i] -gt 0))
            {
                Write-Error "`nDimensions must be positive."
                return
            }
        }
    }

    if ($PSCmdlet.ShouldProcess($Path, "New HDF5 Attribute '$Name'"))
    { 
        if ($Nulll)
        {
            Write-Output(
                New-ItemProperty $Path -Name $Name -ElementType $Type -Null)
        }
        elseif ($Simple) # simple attribute
        {
            if ($Value) {
                Write-Output(
                    New-ItemProperty $Path -Name $Name -ElementType $Type `
                                     -Simple $Simple -Value $Value)
            }
            else {
                Write-Output(
                    New-ItemProperty $Path -Name $Name -ElementType $Type `
                                     -Simple $Simple)
            }
        }
        else
        {
            if ($Scalar) {
                Write-Output(
                    New-ItemProperty $Path -Name $Name -ElementType $Type `
                                     -Value $Scalar)
            }
            else {
                Write-Output(
                    New-ItemProperty $Path -Name $Name -ElementType $Type)
            }
        }
    }
}
