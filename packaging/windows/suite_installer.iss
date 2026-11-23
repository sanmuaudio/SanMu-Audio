; SanMu Audio Windows VST3 4-in-1 Suite Inno Setup Script
; Generates standard Windows 64-bit VST3 Suite installer wizard

#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif
#ifndef MyOutputDir
  #define MyOutputDir "."
#endif
#ifndef MyEq6Dir
  #define MyEq6Dir "VST3\SanMu EQ.vst3"
#endif
#ifndef MyCompDir
  #define MyCompDir "VST3\SanMu Comp.vst3"
#endif
#ifndef MyDelayDir
  #define MyDelayDir "VST3\SanMu Delay.vst3"
#endif
#ifndef MyReverbDir
  #define MyReverbDir "VST3\SanMu Reverb.vst3"
#endif

[Setup]
AppId={{A924FE8B-3651-4191-B72D-SanMuAudioSuite}}
AppName=SanMu Audio Plugin Suite
AppVersion={#MyAppVersion}
AppPublisher=SanMu Audio
AppPublisherURL=https://www.sanmuaudio.com
AppSupportURL=https://www.sanmuaudio.com
AppUpdatesURL=https://www.sanmuaudio.com
DefaultDirName={commoncf64}\VST3
DisableDirPage=no
AppendDefaultDirName=no
DefaultGroupName=SanMu Audio
DisableProgramGroupPage=yes
OutputDir={#MyOutputDir}
OutputBaseFilename=SanMu_Audio_Suite_v{#MyAppVersion}_Windows_x64_Installer
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Full Installation (all 4 plug-ins)"
Name: "custom"; Description: "Custom Installation"

[Components]
Name: "eq6"; Description: "SanMu EQ (Dynamic / Static Equalizer)"; Types: full custom; Flags: checkablealone
Name: "comp"; Description: "SanMu Comp (Master Bus & Vocal Compressor)"; Types: full custom; Flags: checkablealone
Name: "delay"; Description: "SanMu Delay (Stereo Multi-Engine Delay)"; Types: full custom; Flags: checkablealone
Name: "reverb"; Description: "SanMu Reverb (Algorithmic Spatial Reverb)"; Types: full custom; Flags: checkablealone

[Files]
Source: "{#MyEq6Dir}\*"; DestDir: "{app}\SanMu EQ.vst3"; Components: eq6; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyCompDir}\*"; DestDir: "{app}\SanMu Comp.vst3"; Components: comp; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyDelayDir}\*"; DestDir: "{app}\SanMu Delay.vst3"; Components: delay; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyReverbDir}\*"; DestDir: "{app}\SanMu Reverb.vst3"; Components: reverb; Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
english.WelcomeLabel1=Welcome to the SanMu Audio Plugin Suite Setup Wizard
english.WelcomeLabel2=This wizard will install SanMu Audio 64-bit VST3 plug-in suite (SanMu EQ, Comp, Delay, Reverb) on your computer.%n%nIt is recommended that you close your DAW applications before continuing.%n%nClick Next to continue.
