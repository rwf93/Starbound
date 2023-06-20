unit CheckSum;

interface

uses
  Classes, Forms;

type
  TChecksumCRC32 = class
  public
    constructor Create;
    procedure Reset;
    procedure Provide(Data: PByte; Length: Integer);
    function Checksum: Integer;
  private
    FCrc: Integer;
    FCrcTable: array[0..255] of Integer;
  end;

  TFile = class
  public
    FileName: String;
    Missing: Boolean;
    FileSize: Integer;
    Crc: Integer;
  end;

  TChecksums = class
  public
    constructor Create(const PathName: String);
    destructor Destroy; override;

    procedure Scan;
    procedure Load;
    procedure Save;

    procedure Compare(BaseLine: TChecksums; Log: TStrings);
  private
    FFiles: TStringList;
    FCrc: TChecksumCRC32;
    FPathName: String;

    procedure Process(const FileName:  String);
    procedure InnerScan(const PathName: String);
  end;

implementation

uses SysUtils, StrUtils;

constructor TChecksumCRC32.Create;
var
  c, n, k: Integer;
begin
  Reset;
  for n := 0 to 255 do begin
    c := n;
    for k := 0 to 7 do begin
      if c and 1 = 1 then
        c := Integer($edb88320) xor Integer(Cardinal(c) shr 1)
      else
        c := (c shr 1);
    end;
    FCrcTable[n] := c;
  end;
end;

procedure TChecksumCRC32.Reset;
begin
  FCrc := Integer($ffffffff);
end;

procedure TChecksumCRC32.Provide(Data: PByte; Length: Integer);
var
  c, n: Integer;
begin
  c := FCrc;
  for n := 0 to Length-1 do
    c := FCrcTable[(c xor PByte(Integer(Data) + n)^) and $ff] xor Integer(Cardinal(c) shr 8);
  FCrc := c;
end;

function TChecksumCRC32.Checksum: Integer;
begin
  Result := FCrc xor Integer($ffffffff);
end;

constructor TChecksums.Create(const PathName: String);
begin
  FCrc := TChecksumCRC32.Create;
  FFiles := TStringList.Create;
  FFiles.Sorted := True;
  FFiles.Duplicates := dupIgnore;
  FFiles.CaseSensitive := False;
  FPathName := IncludeTrailingPathDelimiter(PathName);
end;

destructor TChecksums.Destroy;
var
  I: Integer;
begin
  for I := 0 to FFiles.Count -1 do
    FFiles.Objects[I].Free;
  FreeAndNil(FFiles);
  FreeAndNil(FCrc);
end;

function FileSize(fileName : wideString) : Int64;
var
  Sr : TSearchRec;
begin
  Result := 0;
  if FindFirst(fileName, faAnyFile, Sr) = 0 then
    Result := Sr.FindData.nFileSizeLow;
  FindClose(Sr) ;
end;

procedure TChecksums.Scan;
begin
  InnerScan(FPathName);
end;

procedure TChecksums.InnerScan(const PathName: String);
var
  Sr : TSearchRec;
begin
  Application.ProcessMessages;
  if FindFirst(PathName + '*', faAnyFile, Sr) = 0 then begin
    try
      repeat
        if not AnsiStartsStr('.', Sr.Name) and not AnsiEndsText('.checksum', Sr.Name) then begin
          if Sr.Attr and faDirectory <> 0 then begin
            InnerScan(IncludeTrailingPathDelimiter(PathName + Sr.Name));
          end else begin
            Process(PathName + Sr.Name);
          end;
        end;
      until FindNext(Sr) <> 0;
    finally
      FindClose(Sr);
    end;
  end;
end;

procedure TChecksums.Process(const FileName:  String);
var
  FS: TFileStream;
  Len: Integer;
  Buffer: array[0..4095] of Byte;
  Entry: TFile;
begin
  Entry := TFile.Create;
  Entry.FileName := AnsiLowerCase(FileName);
  Entry.Missing := not FileExists(Filename);

  Entry.FileSize := 0;
  Entry.Crc := 0;

  if not Entry.Missing then begin
    Entry.FileSize := FileSize(FileName);
    try
      FS := TFileStream.Create(FileName, fmOpenRead or fmShareDenyNone);
      try
        FCrc.Reset;
        while FS.Position <> FS.Size do begin
          Len := FS.Read(Buffer[0], 4096);
          FCrc.Provide(@Buffer[0], Len);
        end;
        Entry.Crc := FCrc.Checksum;
      finally
        FreeAndNil(FS);
      end;
    except
    end;
  end;
  FFiles.AddObject(Entry.FileName, Entry);
end;

procedure Split(const S: String; Parts: TStringList);
begin
  Parts.Clear;
  ExtractStrings(['|'], [], PChar(S), Parts);
end;

procedure TChecksums.Load;
var
  Data: TStringList;
  Parts: TStringList;
  I : Integer;
  Entry: TFile;
begin
  if not FileExists(FPathName+'sb.checksum') then
    Exit;
  Data := TStringList.Create;
  Parts := TStringList.Create;
  Data.LoadFromFile(FPathName+'sb.checksum');
  for I := 0 to Data.Count -1 do begin
    Split(Data.Strings[I], Parts);
    if Parts.Count <> 3 then
      raise Exception.Create('Checksum file parse error');
    Entry := TFile.Create;
    Entry.FileName := FPathName + Parts[0];
    Entry.Missing := False;
    Entry.FileSize := StrToInt(Parts[1]);
    Entry.Crc := StrToInt(Parts[2]);
    FFiles.AddObject(Entry.FileName, Entry);
  end;
  Parts.Free;
  Data.Free;
end;

procedure TChecksums.Save;
var
  Data: TStringList;
  I : Integer;
  Entry: TFile;
  PathName: String;
begin
  PathName := AnsiLowerCase(FPathName);
  Data := TStringList.Create;
  for I := 0 to FFiles.Count -1 do begin
    Entry := TFile(FFiles.Objects[I]);
    if not Entry.Missing then
      Data.Append(AnsiReplaceStr(Entry.FileName, PathName, '') + '|' + IntToStr(Entry.FileSize) + '|0x' + IntToHex(Entry.Crc, 8));
  end;
  Data.SaveToFile(FPathName+'sb.checksum');
  Data.Free;
end;

procedure TChecksums.Compare(BaseLine: TChecksums; Log: TStrings);
var
  I: Integer;
  L: TStringList;
  Base: TFile;
  Local: TFile;
  A: Integer;
begin
  L := TStringList.Create;
  L.Sorted := True;
  L.Duplicates := dupIgnore;
  L.CaseSensitive := False;
  for I := 0 to FFiles.Count-1 do
    L.Add(FFiles.Strings[I]);
  for I := 0 to BaseLine.FFiles.Count-1 do
    L.Add(BaseLine.FFiles.Strings[I]);
  for I := 0 to L.Count-1 do begin
    Base := nil;
    Local := nil;
    if FFiles.Find(L[I], A) then
      Local := TFile(FFiles.Objects[A]);
    if BaseLine.FFiles.Find(L[I], A) then
      Base := TFile(BaseLine.FFiles.Objects[A]);
      
    if Local = nil then
      Log.Append('File ' + L[I] + ' is missing.')
    else if Base = nil then
      Log.Append('File ' + L[I] + ' is not part of the baseline.')
    else if Base.FileSize <> Local.FileSize then
      Log.Append('File ' + L[I] + ' is the wrong size, expected ' +IntToStr(Base.FileSize) + ' but found ' + IntToStr(Local.FileSize) + '.')
    else if Base.Crc <> Local.Crc then
      Log.Append('File ' + L[I] + ' did not match checksum.');
  end;
end;

end.
