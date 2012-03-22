
using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;

[assembly:AssemblyTitleAttribute("PSH5X.dll")];
[assembly:AssemblyDescriptionAttribute("Windows PowerShell Extension for HDF5 : Build")];
#if DEBUG
[assembly: AssemblyConfigurationAttribute("Debug")]
#else
[assembly: AssemblyConfigurationAttribute("Release")]
#endif
[assembly:AssemblyCompanyAttribute("The HDF Group")];
[assembly:AssemblyProductAttribute("PSH5X")];
[assembly:AssemblyCopyrightAttribute("Copyright (C) 2008-2012 The HDF Group. All rights reserved.")];
[assembly:AssemblyTrademarkAttribute("")];
[assembly:AssemblyCultureAttribute("en")];

[assembly:AssemblyVersionAttribute("0.3.*")];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];

[assembly:SecurityPermission(SecurityAction::RequestMinimum, UnmanagedCode = true)];
