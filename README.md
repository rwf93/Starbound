
<h1><img src=https://images.weserv.nl/?url=https://github.com/rwf93/starbound/blob/master/logo.png?raw=true?v=4&h=24&w=24&fit=cover&mask=circle&maxage=7d>Starbound</h1>

A somewhat comprehensive guide on how to build the game from source.

## Build Instructions:
### Windows x86_64
Install [Visual Studio 2023](https://c2rsetup.officeapps.live.com/c2r/downloadVS.aspx?sku=community&channel=Release&version=VS2022&source=VSLandingPage&add=Microsoft.VisualStudio.Workload.ManagedDesktop&add=Microsoft.VisualStudio.Workload.Azure&add=Microsoft.VisualStudio.Workload.NetWeb&includeRecommended=true&cid=2030)  
Install [CMake](https://github.com/Kitware/CMake/releases/download/v3.27.0-rc2/cmake-3.27.0-rc2-windows-x86_64.msi)  
Optionally, Install [QT 5.6.0](https://download.qt.io/new_archive/qt/5.6/5.6.0/qt-opensource-windows-x86-msvc2015_64-5.6.0.exe)  

### Visual Studio

Run ``scripts\windows\setup64.bat``  
A solution should be generated in the ``build`` folder  
Open it and click Build (shortcut: CTRL+B)

### Visual Studio Code
Open a vcvarsall window (x64 Native Tools Command Prompt for VS 2022)  
Run ``scripts\windows\setup64-ninja.bat``  

A ninja project should be generated in the ``build`` folder
Simply run ``ninja``