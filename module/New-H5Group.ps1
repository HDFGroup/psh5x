
Function New-H5Group
{
<#
    .SYNOPSIS
      Create a new HDF5 group 
    .DESCRIPTION
      The New-H5Group function creates one or more new HDF5 groups.
      Use the -Force switch to automatically create missing intermediate
      groups. Unless the current location is on the targeted H5Drive,
      the path(s) must be drive-qualified.
   .PARAMETER Path
     The path of the new HDF5 group(s).
   .PARAMETER TrackAttributeOrder
     Track the attribute creation order.
   .PARAMETER IndexAttributeOrder
     Maintain an index by attribute creation order.
   .PARAMETER TrackLinkOrder
     Track the link creation order.
   .PARAMETER IndexLinkOrder
     Maintain an index by link creation order.
   .PARAMETER Force
     Force the creation of intermediates.
   .EXAMPLE
     New-H5Group -Path h5:\group1
   .EXAMPLE
     New-H5Group -Path h5:\group2\group3 -Force
   .EXAMPLE
     New-H5Group -Path h5:/group1
   .EXAMPLE
     New-H5Group -Path h5:/group1/group2
   .LINK
     New-Item
   .NOTES
     Forward- (/) and backslash (\) seprators are supported in path names.
     The must not be used as part of link names.
 #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param
    (
        [Parameter(Mandatory=$true,
                   HelpMessage='The path of the new HDF5 group(s).')]
        [string[]]
        $Path,
        [Parameter(Mandatory=$false,
                   HelpMessage='Track attribute creation order?')]
        [switch]
        $TrackAttributeOrder,
        [Parameter(Mandatory=$false,
                   HelpMessage='Maintain an index by attribute creation order?')]
        [switch]
        $IndexAttributeOrder,
        [Parameter(Mandatory=$false,
                   HelpMessage='Track link creation order?')]
        [switch]
        $TrackLinkOrder,
        [Parameter(Mandatory=$false,
                   HelpMessage='Maintain an index by link creation order?')]
        [switch]
        $IndexLinkOrder,
        [Parameter(Mandatory=$false,
                   HelpMessage='Force the creation of intermediates?')]
        [switch]
        $Force
    )

    foreach ($p in $Path)
    {
        if (!(Test-Path $p))
        {
            if ($PSCmdlet.ShouldProcess($p, 'New HDF5 Group'))
            { 
                try
                {
                    $param = @('-Path', "'$p'", '-ItemType', 'Group')

                    if ($TrackAttributeOrder) {
                        $param += '-TrackAttributeOrder'
                    }
                    if ($IndexAttributeOrder) {
                        $param += '-IndexAttributeOrder'
                    }
                    if ($TrackLinkOrder) {
                        $param += '-TrackLinkOrder'
                    }
                    if ($IndexLinkOrder) {
                        $param += '-IndexLinkOrder'
                    }
                    if ($Force) {
                        $param += '-Force'
                    }

                    Write-Output(Invoke-Expression "New-Item $param")

                    if (Test-Path $p) {
                        Write-Host "`nSuccess: HDF5 group '$p' created."
                    }
                }
                catch {
                    Write-Debug($_|Out-String)
                    Write-Error "`nUnable to create HDF5 group '$p'. Intermediates missing? Is your current location an H5Drive?"
                }
            }
        }
        else
        {
            Write-Error "`nPath name '$p' is in use."
        }
    }
}
