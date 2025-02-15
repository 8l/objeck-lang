# version string
$version = "3.3.9"

# alternative strings
$version_number = $version.Replace(".", "")
$version_posix = $version -replace "\.\d+$", ("-" + ($version.SubString($version.LastIndexOf(".") + 1)))
$version_posix_long = $version_posix + "-1"
$version_windows = $version.Replace(".", ",")

# update source version header
(Get-Content ..\shared\version.in) | ForEach-Object { $_ -replace "@VERSION@", $version } | ForEach-Object { $_ -replace "@VERSION_NUMBER@", $version_number } | Set-Content ..\shared\version.h
(Get-Content code_doc.in) | ForEach-Object { $_ -replace "@VERSION@", $version } | ForEach-Object { $_ -replace "@VERSION_WINDOWS@", $version_windows } | Set-Content code_doc.cmd

# update debain build files
(Get-Content ..\..\debian\build.in) | ForEach-Object { $_ -replace "@VERSION@", $version_posix } | Set-Content ..\..\debian\build.sh
(Get-Content ..\..\debian\files\changelog.32.in) | ForEach-Object { $_ -replace "@VERSION_LONG@", $version_posix_long } | Set-Content ..\..\debian\files\changelog.32
(Get-Content ..\..\debian\files\changelog.64.in) | ForEach-Object { $_ -replace "@VERSION_LONG@", $version_posix_long } | Set-Content ..\..\debian\files\changelog.64
(Get-Content ..\..\docs\man\obc.in) | ForEach-Object { $_ -replace "@VERSION@", $version_posix_long } | Set-Content ..\..\docs\man\obc.1
(Get-Content ..\..\docs\man\obr.in) | ForEach-Object { $_ -replace "@VERSION@", $version_posix_long } | Set-Content ..\..\docs\man\obr.1
(Get-Content ..\..\docs\man\obd.in) | ForEach-Object { $_ -replace "@VERSION@", $version_posix_long } | Set-Content ..\..\docs\man\obd.1
(Get-Content ..\..\docs\man\obu.in) | ForEach-Object { $_ -replace "@VERSION@", $version_posix_long } | Set-Content ..\..\docs\man\obu.1

# update window resource files
(Get-Content ..\compiler\compiler\objeck.in) | ForEach-Object { $_ -replace "@VERSION@", $version } | ForEach-Object { $_ -replace "@VERSION_WINDOWS@", $version_windows } | Set-Content ..\compiler\compiler\objeck.rc
(Get-Content ..\vm\vm\objeck.in) | ForEach-Object { $_ -replace "@VERSION@", $version } | ForEach-Object { $_ -replace "@VERSION_WINDOWS@", $version_windows } | Set-Content ..\vm\vm\objeck.rc
(Get-Content ..\vm\fcgi\fcgi\objeck.in) | ForEach-Object { $_ -replace "@VERSION@", $version } | ForEach-Object { $_ -replace "@VERSION_WINDOWS@", $version_windows } | Set-Content ..\vm\fcgi\fcgi\objeck.rc
(Get-Content ..\vm\debugger\debugger\objeck.in) | ForEach-Object { $_ -replace "@VERSION@", $version } | ForEach-Object { $_ -replace "@VERSION_WINDOWS@", $version_windows } | Set-Content ..\vm\debugger\debugger\objeck.rc
(Get-Content ..\utilities\utilities\objeck.in) | ForEach-Object { $_ -replace "@VERSION@", $version } | ForEach-Object { $_ -replace "@VERSION_WINDOWS@", $version_windows } | Set-Content ..\utilities\utilities\objeck.rc