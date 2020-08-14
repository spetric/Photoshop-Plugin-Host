//---------------------------------------------------------------------------
#ifndef frmNiGulpUH
#define frmNiGulpUH
#include "TNhelper.h"
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include "hyiedefs.hpp"
#include "hyieutils.hpp"
#include "iesettings.hpp"
#include "ieview.hpp"
#include "iexBitmaps.hpp"
#include "iexLayers.hpp"
#include "iexRulers.hpp"
#include "imageenview.hpp"
#include <Vcl.Menus.hpp>
#include "ieopensavedlg.hpp"
#include <Vcl.Buttons.hpp>
#include "pxCheckLabel.h"
#include "pxSpinSlider.h"
#include <Vcl.Dialogs.hpp>
#include "FolderDialog.hpp"
#include <Vcl.Imaging.pngimage.hpp>
#include "dxGDIPlusClasses.hpp"
//---------------------------------------------------------------------------
class TfrmNiGulp : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *mainMenu;
	TMenuItem *menuFile;
	TMenuItem *itemOpen;
	TMenuItem *itemSave;
	TMenuItem *menuMInteraction;
	TImageEnView *ienView;
	TSpeedButton *btnExecute;
	TPanel *panControl;
	TSpeedButton *btnLoadList;
	TSpeedButton *btnAbout;
	TListBox *plugNameBox;
	TListBox *plugCategoryBox;
	TMenuItem *menuEdit;
	TMenuItem *itemUndo;
	TMenuItem *itemRedo;
	TMenuItem *itemPan;
	TMenuItem *itemZoomInto;
	TMenuItem *N1;
	TMenuItem *itemSelLasso;
	TMenuItem *itemSelPoly;
	TMenuItem *itemSelRect;
	TMenuItem *itemSelEllipse;
	TMenuItem *N2;
	TMenuItem *itemResetZoom;
	TMenuItem *itemFitToScreen;
	TProgressBar *progressPlug;
	TPanel *panEnum;
	TPanel *panSeleSettings;
	TpxSpinSlider *editSelFeather;
	TpxCheckLabel *chkSelRestore;
	TLabel *Label1;
	TpxCheckLabel *chkSelMaxFilter;
	TColorDialog *colorDialog;
	TOpenImageEnDialog *imgOpenDialog;
	TStatusBar *statusBar;
	TPanel *panDir;
	TEdit *editDir;
	TSpeedButton *btnDir;
	TMenuItem *N3;
	TMenuItem *itemSelClear;
	TMenuItem *itemSelMagix;
	TFileOpenDialog *dirOpenDialog;
	TRadioGroup *rgpWandModes;
	TpxSpinSlider *editWandTol;
	TPageControl *pageControl;
	TTabSheet *TabSheet1;
	TTabSheet *TabSheet2;
	TPanel *panFilterSettings;
	TpxCheckLabel *chkScanSubDirs;
	TpxCheckLabel *chkUseByPi;
	TSpeedButton *btnDonate;
	TLabel *Label2;
	TLabel *Label3;
	TMemo *memoHelp;
	TpxCheckLabel *chkFilterROI;
	TSaveImageEnDialog *imgSaveDialog;
	TLabel *Label4;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall itemOpenClick(TObject *Sender);
	void __fastcall btnExecuteClick(TObject *Sender);
	void __fastcall btnLoadListClick(TObject *Sender);
	void __fastcall plugCategoryBoxClick(TObject *Sender);
	void __fastcall plugNameBoxClick(TObject *Sender);
	void __fastcall itemUndoClick(TObject *Sender);
	void __fastcall itemRedoClick(TObject *Sender);
	void __fastcall itemMouseInteractClick(TObject *Sender);
	void __fastcall itemResetZoomClick(TObject *Sender);
	void __fastcall itemFitToScreenClick(TObject *Sender);
	void __fastcall btnAboutClick(TObject *Sender);
	void __fastcall btnDirClick(TObject *Sender);
	void __fastcall editWandTolValueChange(TObject *Sender);
	void __fastcall chkSelMaxFilterValueChange(TObject *Sender);
	void __fastcall rgpWandModesClick(TObject *Sender);
	void __fastcall btnDonateClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall itemSaveClick(TObject *Sender);
private:	// User declarations
	TIEBitmap *srcImage, *srcAlpha, *srcMask;
	String plugSelected;
	TStringList *listCategory;
	TList *listPlugData;
	bool fragBitmap;
    String exePath, cfgFile, startFilterPath;
	void __fastcall plugEnumNotify(TNhEnumPlug *);
	void __fastcall plugProgressNotify(unsigned int done, unsigned int total);
    bool __fastcall plugColorPickerNotify(unsigned int &pickedColor);
	void __fastcall plugSetImage(void);
	void __fastcall plugSetMask(void);
	void __fastcall plugDeleteMask(void);
    void __fastcall plugSetROI(int top = 0, int left = 0, int bottom = 0, int right = 0);
	void __fastcall plugDeleteDataList(void);

public:		// User declarations
	__fastcall TfrmNiGulp(TComponent* Owner);
	__fastcall ~TfrmNiGulp();
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmNiGulp *frmNiGulp;
//---------------------------------------------------------------------------
#endif
