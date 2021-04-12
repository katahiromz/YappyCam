; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "YappyCam"
#define MyAppVersion "0.9.2"
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
OutputBaseFilename=YappyCam32-0.9.2-setup
Compression=lzma
SolidCompression=yes
OutputDir=.

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "build\YappyCam.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\finalize.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\sound2wav.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\silent.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "READMEJP.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "HISTORY.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libgfortran-5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libHalf-2_4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libIex-2_4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libIlmImf-2_4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libIlmThread-2_4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libImath-2_4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libjasper-4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libjpeg-8.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\liblzma-5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopenblas.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_calib3d412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_core412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_features2d412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_flann412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_imgcodecs412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_imgproc412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_objdetect412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libopencv_videoio412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libpng16-16.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libquadmath-0.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libtiff-5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libwebp-7.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\libzstd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\opencv_videoio_ffmpeg412.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\openh264-1.8.0-win32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\tbb.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\msys2\x86\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "haarcascade_frontalface_default.xml"; DestDir: "{app}"; Flags: ignoreversion
Source: "haarcascade_frontalface_alt.xml"; DestDir: "{app}"; Flags: ignoreversion
Source: "lbpcascade_frontalface_improved.xml"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Brightness.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\ChromaKey.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Clock.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\ColorInvert.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\DecoFace.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\DecoFrame.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\FaceDetection.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\GaussianBlur.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Monochrome.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Mosaic.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Noize.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Quadruple.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Quarter.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Resizing.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Rotation.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Sepia.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Trimming.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\medianBlur.yap"; DestDir: "{app}"; Flags: ignoreversion
Source: "plugins\DecoFace\CatEar.png"; DestDir: "{app}\DecoFace"; Flags: ignoreversion
Source: "plugins\DecoFace\FaceModel.png"; DestDir: "{app}\DecoFace"; Flags: ignoreversion
Source: "plugins\DecoFrame\GoldFrame.png"; DestDir: "{app}\DecoFrame"; Flags: ignoreversion
Source: "plugins\DecoFrame\SilverFrame.png"; DestDir: "{app}\DecoFrame"; Flags: ignoreversion
Source: "plugins\DecoFrame\Stars.png"; DestDir: "{app}\DecoFrame"; Flags: ignoreversion
Source: "plugins\DecoFrame\WoodFrame.png"; DestDir: "{app}\DecoFrame"; Flags: ignoreversion
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
