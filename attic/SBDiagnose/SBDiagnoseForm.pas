unit SBDiagnoseForm;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls, CheckSum, ComCtrls;

type
  TForm1 = class(TForm)
    Log: TMemo;
    Timer1: TTimer;
    ProgressBar1: TProgressBar;
    procedure Timer1Timer(Sender: TObject);
  private
    procedure ScanBaseInformation;
    procedure ScanSteamInformation;
    procedure ScanSteamOverlay;
    procedure ScanCompatFlags;
    procedure ScanHsSrvDll;
    procedure ScanChecksums;
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

uses
  Registry, StrUtils;

{$R *.dfm}

function GetEnvVarValue(const VarName: String): String;
var
  BufSize: Integer;
begin
  BufSize := GetEnvironmentVariable(PChar(VarName), nil, 0);
  if BufSize > 0 then
  begin
    SetLength(Result, BufSize - 1);
    GetEnvironmentVariable(PChar(VarName), PChar(Result), BufSize);
  end
  else
    Result := '';
end;

function ErrorMessage: String;
var
  Error: DWORD;
begin
  Error := GetLastError;
  Result := IntToStr(Error) + ' ' + SysErrorMessage(Error);
end;

procedure TForm1.ScanBaseInformation;
var
  Handle: HMODULE;
  ProcessAffinityMask: DWORD;
  SystemAffinityMask: DWORD;
begin
  Log.Lines.Append('Number of processors: '+GetEnvVarValue('NUMBER_OF_PROCESSORS'));
  Log.Lines.Append('Processor architecture: '+GetEnvVarValue('PROCESSOR_ARCHITECTURE'));
  Log.Lines.Append('Processor: '+GetEnvVarValue('PROCESSOR_IDENTIFIER'));

  Handle := GetCurrentProcess;
  if not GetProcessAffinityMask(Handle, ProcessAffinityMask, SystemAffinityMask) then begin
    Log.Lines.Append('ProcessAffinityMask: Failed to query, error: '+ErrorMessage);
  end
  else begin
    Log.Lines.Append('ProcessAffinityMask: '+IntToHex(Integer(ProcessAffinityMask), 8));
    Log.Lines.Append('SystemAffinityMask: '+IntToHex(Integer(SystemAffinityMask), 8));
  end;

  Log.Lines.Append('WindowsVersion: '+IntToStr(Win32MajorVersion)+'.'+IntToStr(Win32MinorVersion)+'.'+IntToStr(Win32BuildNumber)+' '+Win32CSDVersion);

  if (Win32MajorVersion < 5) or ((Win32MajorVersion = 5) and (Win32MinorVersion < 1)) then begin
    Log.Lines.Append('Versions of windows before Windows XP are not supported.');
  end;
  if (Win32MajorVersion = 5) and (Win32MinorVersion > 0) then begin
    Log.Lines.Append('Windows XP detected, it is recommended to use a more recent version of windows.');
  end;

  Log.Lines.Append('');
end;

procedure TForm1.ScanSteamInformation;
begin
  Log.Lines.Append('EnvAppId: '+GetEnvVarValue('SteamAppId'));
  Log.Lines.Append('');
end;

procedure TForm1.ScanSteamOverlay;
var
  Handle: HMODULE;
begin
  Handle := GetModuleHandle('GameOverlayRenderer');
  if Handle <> 0 then begin
    Log.Lines.Append('The Steam Overlay has been detected.');
    Log.Lines.Append('Disabling the Steam overlay may improve render performance.');
    Log.Lines.Append('http://community.playstarbound.com/index.php?threads/sbdiagnose-faq.52470/');
    Log.Lines.Append('');
  end;
end;

procedure TForm1.ScanCompatFlags;
var
  Registry: TRegistry;
  Steam: Boolean;
  Starbound: Boolean;
  procedure Scan;
  var
    I: Integer;
    Entries: TStringList;
  begin
    Entries := TStringList.Create;
    if Registry.OpenKey('Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers\', False) then begin
      Registry.GetValueNames(Entries);
      for I := 0 to Entries.Count-1 do begin
        if AnsiContainsText(Entries[i], '\steam.exe') then
          Steam := True;
        if AnsiContainsText(Entries[i], '\starbound.exe') then
          Starbound := True;
        if AnsiContainsText(Entries[i], '\starbound_opengl.exe') then
          Starbound := True;
        if AnsiContainsText(Entries[i], '\starbound\win32\') then
          Starbound := True;
      end;
      Registry.CloseKey;
    end;
    Entries.Free;
    Registry.Free;
  end;
begin
  if (GetEnvVarValue('__COMPAT_LAYER') <> '') then begin
    Log.Lines.Append('Environment Compatibility Flags: '+GetEnvVarValue('__COMPAT_LAYER'));
    Log.Lines.Append('');
  end;

  Steam := False;
  Starbound := False;

  Registry := TRegistry.Create;
  Registry.RootKey := HKEY_CURRENT_USER;
  Scan;

  Registry := TRegistry.Create;
  Registry.RootKey := HKEY_LOCAL_MACHINE;
  Scan;

  Registry := TRegistry.Create;
  Registry.RootKey := HKEY_CURRENT_USER;
  Registry.Access := $000f013f;
  Scan;

  Registry := TRegistry.Create;
  Registry.RootKey := HKEY_LOCAL_MACHINE;
  Registry.Access := $000f013f;
  Scan;

  if Steam then begin
    Log.Lines.Append('Steam is running in compatibility mode.');
    Log.Lines.Append('Running steam in compatbility mode can cause stability and performance problems.');
    Log.Lines.Append('http://community.playstarbound.com/index.php?threads/sbdiagnose-faq.52470/');
    Log.Lines.Append('');
  end;
  if Starbound then begin
    Log.Lines.Append('Starbound is running in compatibility mode.');
    Log.Lines.Append('Running starbound in compatbility mode can cause stability and performance problems.');
    Log.Lines.Append('http://community.playstarbound.com/index.php?threads/sbdiagnose-faq.52470/');
    Log.Lines.Append('');
  end;
end;

procedure TForm1.ScanHsSrvDll;
var
  Handle: HMODULE;
begin
  Handle := LoadLibraryEx('hssrv.dll', 0, LOAD_LIBRARY_AS_DATAFILE);
  if Handle = 0 then
    Handle := GetModuleHandle('HsSrv');
  if Handle <> 0 then begin
    Log.Lines.Append('HsSrv.dll detected (Asus Xonar Drivers).');
    Log.Lines.Append('There have been reports of this driver having compatibility issues with starbound.');
    Log.Lines.Append('http://community.playstarbound.com/index.php?threads/sbdiagnose-faq.52470/');
    Log.Lines.Append('');
  end;
end;

procedure TForm1.ScanChecksums;
var
  A, B: TChecksums;
  F: String;
begin
  F := IncludeTrailingPathDelimiter(ExtractFilePath(Application.ExeName)) + '..\assets\';
  Log.Lines.Add('Scanning checksums...');
  Application.ProcessMessages;
  try
    if not FileExists(F + 'sb.checksum') then begin
      Log.Lines.Add('No baseline found, creating.');
      Application.ProcessMessages;
      A := TChecksums.Create(F);
      A.Scan;
      A.Save;
      A.Free;
    end else begin
      A := TChecksums.Create(F);
      A.Scan;
      Log.Lines.Add('Loading baseline.');
      Application.ProcessMessages;
      B := TChecksums.Create(F);
      B.Load;

      Log.Lines.Add('Comparing checksums.');
      Application.ProcessMessages;
      A.Compare(B, Log.Lines);

      A.Free;
    end;
  except
    on E: Exception do
      Log.Lines.Add('Failed to scan checksums: ' + E.ClassName + ': ' + E.Message);
  end;
  Log.Lines.Add('done.');
  Log.Lines.Add('');
end;

procedure TForm1.Timer1Timer(Sender: TObject);
begin
  Timer1.Enabled := False;
  Log.Lines.Append('Scanning System');
  Log.Lines.Append('');
  Application.ProcessMessages;

  ProgressBar1.Position := 10;
  Application.ProcessMessages;

  ScanChecksums;

  ProgressBar1.Position := 50;
  Application.ProcessMessages;

  ScanBaseInformation;

  ProgressBar1.Position := 60;
  Application.ProcessMessages;

  ScanSteamInformation;

  ProgressBar1.Position := 70;
  Application.ProcessMessages;

  ScanSteamOverlay;

  ProgressBar1.Position := 80;
  Application.ProcessMessages;

  ScanCompatFlags;

  ProgressBar1.Position := 90;
  Application.ProcessMessages;

  ScanHsSrvDll;

  ProgressBar1.Position := ProgressBar1.Max;
  ProgressBar1.Enabled := False;
  Log.Lines.Append('Done.');
end;

end.
