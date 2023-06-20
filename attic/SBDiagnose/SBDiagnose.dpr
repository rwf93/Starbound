program SBDiagnose;

uses
  Forms,
  SBDiagnoseForm in 'SBDiagnoseForm.pas' {Form1},
  CheckSum in 'CheckSum.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.Title := 'Starbound Diagnostics';
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
