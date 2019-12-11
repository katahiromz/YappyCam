; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "YappyCam"
#define MyAppVersion "0.9.1"
#define MyAppPublisher "Katayama Hirofumi MZ"
#define MyAppURL "https://katahiromz.web.fc2.com"
#define MyAppExeName "YappyCam.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{3EC5E49D-0FA3-4102-AF34-CE71816FEA53}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=LICENSE.txt
OutputBaseFilename=YappyCam32-0.9.1-setup
Compression=lzma
SolidCompression=yes
OutputDir=.

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "YappyCam.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "finalize.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "sound2wav.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "silent.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "READMEJP.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libHalf-2_3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libIex-2_3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libIlmImf-2_3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libIlmThread-2_3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libImath-2_3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libgfortran-5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libjasper-4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libjpeg-8.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\liblzma-5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopenblas.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_core411.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_imgcodecs411.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_imgproc411.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_videoio411.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libpng16-16.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libquadmath-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libtiff-5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libwebp-7.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libzstd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\opencv_videoio_ffmpeg411.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\tbb.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\openh264-1.8.0-win32.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}\YappyCam"; Filename: "{app}\{#MyAppExeName}"
Name: "{commonprograms}\{#MyAppName}\{cm:UninstallProgram,YappyCam}"; Filename: "{app}\unins000.exe"
Name: "{commonprograms}\{#MyAppName}\README.txt"; Filename: "{app}\README.txt"
Name: "{commonprograms}\{#MyAppName}\READMEJP.txt"; Filename: "{app}\READMEJP.txt"
Name: "{commonprograms}\{#MyAppName}\LICENSE.txt"; Filename: "{app}\LICENSE.txt"
Name: "{commondesktop}\YappyCam"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
