
Function Show-H5Drive
{
<#
    .SYNOPSIS
      Creates a DOT language representation of an HDF5 drive
    .DESCRIPTION
      The Show-H5Drive cmdlet creates a DOT language representation
      of an HDF5 drive which can be used to create an image file
      using a tool such as GraphViz.
    .PARAMETER Name
      The name of the HDF5 drive
    .LINK
      http://www.graphviz.org/
       http://www.graphviz.org/content/dot-language
#>
    [CmdletBinding(SupportsShouldProcess=$false)]
    param
    (
        [Parameter(Mandatory=$true,
                   Position=0,
                   HelpMessage='The name of an HDF5 drive')]
        [ValidateNotNull()]
        [string]
        $Name
    )

    if (-not (Get-PSDrive -PSProvider HDF5 | % {$_.Name}) -contains `
        $Name) {
        Write-Warning "H5Drive '$Name' not found!"
        continue
    }
    $root = "$($Name):\"

    $ht = @{}

    $address = (Get-Item $root -Detailed).Address
    $ht[$address] = 'G'

    $groups = @()
    $dsets = @()
    $dtypes = @()

    # get objects first

    Get-ChildItem $root -Recurse -Detailed -Filter dgt | `
        %{ $ht[$_.Address] = switch ($_.ItemType) {
            Group          {'G'; break}
            Dataset        {'D'; break}
            DatatypeObject {'T'; break}
            default        {'U'; break} }
        }

    $gnodes = '    node [shape = circle]; '
    $dnodes = '    node [shape = rect]; '
    $tnodes = '    node [shape = triangle]; '

    foreach ($k in $ht.keys)
    {
        switch ($ht[$k]) {
            G { $gnodes += " G_$($k)"; break}
            D { $dnodes += " D_$($k)"; break}
            T { $tnodes += " T_$($k)"; break}
        }
    }
    $gnodes += ';'
    if (-not $dnodes.Trim().EndsWith(';')) { $dnodes += ';'}
    if (-not $tnodes.Trim().EndsWith(';')) { $tnodes += ';'}

    # symbolic links are different

    $symlinks = @{}
    $count = 0
    Get-ChildItem $root -Recurse -Detailed -Filter -dgt | `
        %{ $key = $_.H5PathName; switch ($_.ItemType) {
            SoftLink { $symlinks[$key] = "S_$($count)"; ++$count; break}
        }}

    # print the DOT output

@"
digraph $Name {
rankdir = LR;
$gnodes
$dnodes
$tnodes
$slinks
"@
    foreach ($k in $symlinks.Keys) {
        "    node [shape = plaintext, label = `"$($k)`"]; $($symlinks[$k]);"
    }

    #print edges

    $items = (Get-ChildItem $root -Recurse -Detailed)
    if ($items -ne $Null)
    {
        foreach ($i in $items)
        {
            $src = "G_$((Get-Item $i.PSParentPath -Detailed).Address)"
            $label = "$($i.PSChildName)"
    
            if ($i.ItemType -ne 'SoftLink') {
                $address = (Get-Item $i.PSPath -Detailed).Address
                $dst = "$($ht[$address])_$address"
                "    $src -> $dst [ label = `"$label`" ];"
            }
            else {
                "    $src -> $($symlinks[$i.H5PathName]) [ label = `"$label`" ];"
            }
        }
    }

    # Can't go without this one!

    '}'
}
