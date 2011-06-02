
param( `
  [string] $Configuration = $(throw write-host `
    "Please specify a configuration [Debug, Release]." --Foregroundcolor Red))

$myDir = Split-Path -Parent $MyInvocation.MyCommand.Path

$dll = $myDir + '\x64\'

if (($Configuration.ToUpper() -eq "DEBUG") `
     -or ($Configuration.ToUpper() -eq "RELEASE"))
{
  $dll = $dll + $Configuration + "\PSH5X.dll" 
}
else
{
  throw write-host "Invalid configuration $Configuration specified!" `
    "Valid configurations are Debug and Release." --Foregroundcolor Red
}

$installutil = $env:windir + '\Microsoft.NET\'

if ($env:PROCESSOR_ARCHITECTURE.ToUpper() -eq "X86")
{
  $installutil = $installutil + 'Framework\v4.0.30319\InstallUtil.exe'
}
elseif ($env:PROCESSOR_ARCHITECTURE.ToUpper() -eq "AMD64")
{
  $installutil = $installutil + 'Framework64\v4.0.30319\InstallUtil.exe'
}
else
{
  throw write-host "Unable to determine processor architecture!" --Foregroundcolor Red
}

# remove-pssnapin HDF5PSSnapIn -passthru

write-host "Unregistering PSHDFX snap-in"
& $installutil '/u' $dll
