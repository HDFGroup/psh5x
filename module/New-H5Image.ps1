
Function New-H5Image
{
<#
    .SYNOPSIS
      Creates a new HDF5 image 
    .DESCRIPTION
      The New-H5Image function creates new HDF5 image.
    .PARAMETER Path
      The path name of the new HDF5 image 
    .PARAMETER FromFile 
      The file name of an image file (JPEG, GIF, PNG)
    .PARAMETER Bits 
      The number of bits per pixel, 8 or 24.
    .PARAMETER Width
      The image width
    .PARAMETER Height
      The image height
    .PARAMETER Interlace 
      The interlace mode, by 'Plane' or by 'Pixel'
    .PARAMETER Force 
      Force the creation of intermediate groups
    .EXAMPLE
      New-H5Image h5:/image C:\tmp\foo.gif
 #>
    [CmdletBinding(SupportsShouldProcess=$true,DefaultParameterSetName='File')]
    param
    (
        [Parameter(Mandatory=$true,
                   Position=0,
                   HelpMessage='The path name of the new HDF5 Image')]
        [string]
        $Path,
        [Parameter(Mandatory=$true,
                   Position=1,
                   ParameterSetName='File',
                   HelpMessage='The file name of an image file')]
        [ValidateNotNull()]
        [string]
        $FromFile,
        [Parameter(Mandatory=$true,
                   Position=1,
                   ParameterSetName='NoFile',
                   HelpMessage='Bits per pixel [8,24]')]
        [ValidateSet(8,24)]
        [int]
        $Bits,
        [Parameter(Mandatory=$true,
                   Position=2,
                   ParameterSetName='NoFile',
                   HelpMessage='Image width')]
        [ValidateScript({$_ -gt 0})]
        [long]
        $Width,
        [Parameter(Mandatory=$true,
                   Position=3,
                   ParameterSetName='NoFile',
                   HelpMessage='Image height')]
        [ValidateScript({$_ -gt 0})]
        [long]
        $Height,
        [Parameter(Mandatory=$true,
                   Position=4,
                   ParameterSetName='NoFile',
                   HelpMessage='Interlace mode')]
        [ValidateSet('Plane','Pixel')]
        [string]
        $Interlace,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    if((Test-Path $Path))
    {
        Write-Error "`nThe path name '$Path' is in use."
        return
    }
    if ($FromImage)
    {
        if (!(Test-Path $FromFile))
        {
            Write-Error "`nImage file '$FromFile' not found."
            return
        }
    }

    if ($PSCmdlet.ShouldProcess($Path, 'New HDF5 image'))
    { 
        try
        {
            if ($FromFile)
            {
                if ($Force) {
                    Write-Output(
                        New-Item $Path -ItemType Image -Value $FromFile -Force)
                }
                else {
                    Write-Output(
                        New-Item $Path -ItemType Image -Value $FromFile)
                }
            }
            else
            {
                if ($Force) {
                    Write-Output(New-Item $Path -ItemType Image `
                                          -Bits $Bits `
                                          -WxH @($Width,$Height) `
                                          -InterlaceMode $Interlace -Force)
                }
                else {
                    Write-Output(New-Item $Path -ItemType Image `
                                          -Bits $Bits `
                                          -WxH @($Width,$Height) `
                                          -InterlaceMode $Interlace)
                }
            }
            if (Test-Path $Path) {
                Write-Host "`nSuccess: HDF5 image '$Path' created."
            }
        }
        catch
        {
            Write-Debug ($_|Out-String)
            Write-Error "`nCreation of HDF5 image '$Path' failed."
        }
    }
}
