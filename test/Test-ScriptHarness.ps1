#
# Ed Wilson, Windows PowerShell 2.0 - Best Practices, p. 503
#

Import-Module HDF5

$path = Join-Path (Split-Path $MyInvocation.MyCommand.Path) ps1
$report = [IO.Path]::GetTempFileName()

Get-ChildItem -Path $path -Include *.ps1 -Recurse |
ForEach-Object -Begin `
{
    $stime = Get-Date
    $ErrorActionPreference = "SilentlyContinue"
    "Testing ps1 scripts in $path $stime" |
        Out-File -Append -FilePath $report

} -Process `
{
    $error.Clear()
    "`n" |
        Out-File -Append -FilePath $report
    $startTime = Get-Date
    "  Begin testing $_ at $startTime" |
        Out-File -Append -FilePath $report
    Invoke-Expression -Command $_
    $endTime = Get-Date
    "  End testing $_ at $endTime" |
        Out-File -Append -FilePath $report
    "`n" |
        Out-File -Append -FilePath $report
    "  Script generated $($error.Count) errors" |
        Out-File -Append -FilePath $report
    "`n" |
        Out-File -Append -FilePath $report
    "  Elapsed time $($endTime - $startTime)" |
        Out-File -Append -FilePath $report

} -End `
{
    $etime = Get-Date
    $ErrorActionPreference = "Continue"
    "`n" |
        Out-File -Append -FilePath $report
    "Completed testing all scripts in $path $etime" |
        Out-File -Append -FilePath $report
    "Testing took $($etime - $stime)" |
        Out-File -Append -FilePath $report
}

Notepad $report
