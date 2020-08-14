//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "frmNoPressU.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmNoPress *frmNoPress;
//---------------------------------------------------------------------------
__fastcall TfrmNoPress::TfrmNoPress(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmNoPress::SpeedButton1Click(TObject *Sender)
{
// what? I have a site to maintain! And what about development software and libraries?
String url = "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LTLAUA3TYDQMJ";
ShellExecute(0, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
Close();
}
//---------------------------------------------------------------------------
void __fastcall TfrmNoPress::SpeedButton2Click(TObject *Sender)
{
Close();
}
//---------------------------------------------------------------------------

