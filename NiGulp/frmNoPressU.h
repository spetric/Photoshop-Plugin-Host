//---------------------------------------------------------------------------

#ifndef frmNoPressUH
#define frmNoPressUH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
//---------------------------------------------------------------------------
class TfrmNoPress : public TForm
{
__published:	// IDE-managed Components
	TSpeedButton *SpeedButton1;
	TPanel *Panel1;
	TSpeedButton *SpeedButton2;
	TMemo *Memo1;
	void __fastcall SpeedButton2Click(TObject *Sender);
	void __fastcall SpeedButton1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TfrmNoPress(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmNoPress *frmNoPress;
//---------------------------------------------------------------------------
#endif
