
Function Check-HDF5Snapin
{
    $count = 0
    Get-PSSnapin -Registered | Where-Object { $_.Name -eq 'HDF5' } | ForEach-Object {$count++}
    if ($count -ne 1) {
        Throw 'The HDF5 plugin has not been registered on this machine!'
    }

    $count = 0
    Get-PSSnapin | Where-Object { $_.Name -eq 'HDF5' } | ForEach-Object {$count++}
    if ($count -ne 1) {
        Write-Host 'HDF5 snap-in not found. Loading...'; Add-PSSnapin HDF5
    }

    $count = 0
    Get-PSSnapin | Where-Object { $_.Name -eq 'HDF5' } | ForEach-Object {$count++}
    if ($count -ne 1) {
        Throw 'Unable to load HDF5 plugin!'
    }
}

