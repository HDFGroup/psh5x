
using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;

[assembly:AssemblyTitleAttribute("PSH5X.dll")];
[assembly:AssemblyDescriptionAttribute("PowerShell Extension for HDF5")];
#if DEBUG
[assembly: AssemblyConfigurationAttribute("Debug")]
#else
[assembly: AssemblyConfigurationAttribute("Release")]
#endif
[assembly:AssemblyCompanyAttribute("The HDF Group")];
[assembly:AssemblyProductAttribute("PSH5X")];
[assembly:AssemblyCopyrightAttribute("Copyright (C) The HDF Group 2008-2011. All rights reserved.")];
[assembly:AssemblyTrademarkAttribute("")];
[assembly:AssemblyCultureAttribute("en")];

[assembly:AssemblyVersionAttribute("0.2.*")];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];

[assembly:SecurityPermission(SecurityAction::RequestMinimum, UnmanagedCode = true)];
