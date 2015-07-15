
Function Check-OpenHandles
{
    $drives = Get-PSDrive -PSProvider HDF5
    foreach ($d in $drives)
    {
        $count = $d.OpenAttributeCount
        if ($count -gt 0)
        {
            Throw "Drive $d has a non-zero open attribute count of $count !!!"
        }
        $count = $d.OpenDatasetCount
        if ($count -gt 0)
        {
            Throw "Drive $d has a non-zero open dataset count of $count !!!"
        }
        $count = $d.OpenDatatypeObjectCount
        if ($count -gt 0)
        {
            Throw "Drive $d has a non-zero open datatype count of $count !!!"
        }
        $count = $d.OpenFileCount
        if ($count -ne 1)
        {
            Throw "Drive $d has a non-zero open file count of $count !!!"
        }
        $count = $d.OpenGroupCount
        if ($count -gt 0)
        {
            Throw "Drive $d has a non-zero open group count of $count !!!"
        }
    }
}

