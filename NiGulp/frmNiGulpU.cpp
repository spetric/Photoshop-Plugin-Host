//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "frmNiGulpU.h"
#include "pspiHost.h"
#include "FileCtrl.hpp"
#include "frmNoPressU.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "hyiedefs"
#pragma link "hyieutils"
#pragma link "iesettings"
#pragma link "ieview"
#pragma link "iexBitmaps"
#pragma link "iexLayers"
#pragma link "iexRulers"
#pragma link "imageenview"
#pragma link "ieopensavedlg"
#pragma link "pxCheckLabel"
#pragma link "pxSpinSlider"
#pragma link "FolderDialog"
#pragma link "dxGDIPlusClasses"
#pragma resource "*.dfm"
TfrmNiGulp *frmNiGulp;
//---------------------------------------------------------------------------
__fastcall TfrmNiGulp::TfrmNiGulp(TComponent* Owner)
	: TForm(Owner)
{
IEGlobalSettings()->AutoFragmentBitmap = false;
IEGlobalSettings()->MsgLanguage = msEnglish;
//IEGlobalSettings()->EnableTheming = true;
//
String platver;
#ifdef __WIN32__
	platver = " ver.0.9 (32-bit)";
#else
	platver = " ver.0.9 (64-bit)";
#endif
this->Caption = this->Caption + platver;
//
listCategory = new TStringList(this);
listCategory->Sorted = true;
listCategory->Duplicates = System::Types::TDuplicates::dupIgnore;
//
listPlugData = new TList();
SetExceptionMask(exAllArithmeticExceptions); // some filters need that
//
exePath = IncludeTrailingBackslash(ExtractFilePath(Application->ExeName));
cfgFile = exePath + "NiGulp.cfg";
startFilterPath = "";
}
//---------------------------------------------------------------------------
__fastcall TfrmNiGulp::~TfrmNiGulp()
{
plugDeleteMask();
plugDeleteDataList();
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::FormShow(TObject *Sender)
{
pageControl->ActivePageIndex = 0;
itemMouseInteractClick(itemPan);
ienView->Blank();
ienView->Proc->UndoLimit = 10;
ienView->SelectionMaskDepth = 8;
// private TIEBitmaps
srcImage = 0;
srcAlpha = 0;
srcMask = 0;
//
ienView->MagicWandMaxFilter = chkSelMaxFilter->Checked;
ienView->MagicWandMode      = (TIEMagicWandMode)(rgpWandModes->ItemIndex);
ienView->MagicWandTolerance = editWandTol->IntValue;
//
fragBitmap = IEGlobalSettings()->AutoFragmentBitmap;
//
plugSelected = L"";
if (FileExists(cfgFile))
   {
   TStringList *cfg = new TStringList();
   cfg->LoadFromFile(cfgFile);
   if (cfg->Count > 0)
	  {
	  editDir->Text = cfg->Strings[0];
      startFilterPath = cfg->Strings[0];
	  }
   delete cfg;
   }
else
   {
   editDir->Text =  exePath + "8bf filters";
   startFilterPath = editDir->Text;
   }
//
pspiSetImageOrientation(PSPI_IMG_ORIENTATION_INVERT);
// set our enumerate call-back notifier
TNhelper::nhPlugEnumNotify = plugEnumNotify;
// set our progress call-back notifier
TNhelper::nhPlugProgressNotify = plugProgressNotify;
// set our color picker call-back notifier
TNhelper::nhPlugColorPickerNotify = plugColorPickerNotify;
// set progress call back function (in TNhelper)
pspiSetProgressCallBack(TNhelper::nhProgress);
// set color picker call back function (in TNhelper)
pspiSetColorPickerCallBack(TNhelper::nhColorPicker);
//
btnLoadListClick(this);
//
ienView->SetFocus();
//
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::FormClose(TObject *Sender, TCloseAction &Action)
{
if (startFilterPath.Trim() == editDir->Text.Trim())
   return;
if (MessageDlg("Do you want to save current filter's folder to NiGulp.cfg?", mtConfirmation, TMsgDlgButtons() << mbYes<<mbNo, 0) == mrYes)
   {
   TStringList *cfg = new TStringList();
   cfg->Add(editDir->Text.Trim());
   cfg->SaveToFile(cfgFile);
   delete cfg;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugEnumNotify(TNhEnumPlug *plugEnumData)
{
// notifications from enumerate call-back - collecting plug data
// add category to orderd string list
listCategory->Add(plugEnumData->category.Trim());
TNhEnumPlug *plugData = new TNhEnumPlug();
plugData->category = plugEnumData->category;
plugData->name = plugEnumData->name;
plugData->entrypoint = plugEnumData->entrypoint;
plugData->fullpath = plugEnumData->fullpath;
listPlugData->Add((void*)plugData);
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugProgressNotify(unsigned int done, unsigned int total)
{
// notifications from progress call-back - show plug progress
double pos = (double)done * progressPlug->Max / (double)total;
progressPlug->Position = (int)pos;
}
//---------------------------------------------------------------------------
bool __fastcall TfrmNiGulp::plugColorPickerNotify(unsigned int &pickedColor)
{
// notifications from color picker call-back - pick color
bool lRet = colorDialog->Execute();
if (lRet)
	{
	unsigned int cpc = (unsigned int)(colorDialog->Color);
    pickedColor = cpc;
	}
return lRet;
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugSetImage(void)
{
int w, h, iStride, aStride = 0;
void *iBuf, *aBuf = 0;
srcImage = ienView->IEBitmap;
srcAlpha = 0;
TImgType type;
if (srcImage->PixelFormat == ie24RGB)
   type = PSPI_IMG_TYPE_BGR;
else if (srcImage->PixelFormat == ie8g)
   type = PSPI_IMG_TYPE_GRAY;
else
  {
  // TODO: convert to ie24RGB...or something.
  }
w = srcImage->Width;
h = srcImage->Height;
if (fragBitmap) // if bitmap is fragmented add scanlines in loop
   {
   // orientation set by pspiSetImageOrientation does not have effect when scanlines are added one by one
   pspiStartImageSL(type, w, h, srcImage->HasAlphaChannel);
   for (int i = 0; i < h; i++)
		pspiAddImageSL(srcImage->ScanLine[i], srcImage->HasAlphaChannel ? srcAlpha->ScanLine[i] : 0);
   pspiFinishImageSL();
   }
else    // contiguous bitmap buffer
   {
   iStride = srcImage->RowLen;
   iBuf = srcImage->ScanLine[h - 1];
   if (srcImage->HasAlphaChannel)
	  {
	  srcAlpha = srcImage->AlphaChannel;
	  aStride = srcAlpha->RowLen;
	  aBuf = srcAlpha->Scanline[h - 1];
	  }
   pspiSetImage(type, w, h, iBuf, iStride, aBuf, aStride);
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugSetMask(void)
{
int mw = srcMask->Width;
int mh = srcMask->Height;
if (fragBitmap) // if bitmap is fragmented add scanlines in lopp
   {
   // orientation set by pspiSetImageOrientation does not have effect when scanlines are added one by one
   pspiStartMaskSL(mw, mh, chkUseByPi->Checked);
   for (int i = 0; i < mh; i++)
		pspiAddMaskSL(srcMask->ScanLine[i]);
   pspiFinishMaskSL();
   }
else   // contiguous bitmap buffer
	{
	pspiSetMask(mw, mh, srcMask->Scanline[mh - 1], srcMask->RowLen, chkUseByPi->Checked);
	}
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugDeleteMask(void)
{
pspiSetMask();  // no params, releases mask
if (srcMask)
   {
   delete srcMask;
   srcMask = 0;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugSetROI(int top, int left, int bottom, int right)
{
pspiSetRoi(top, left, bottom, right);
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugDeleteDataList(void)
{
for (int i = 0; i < listPlugData->Count; i++)
	{
	TNhEnumPlug *plug = (TNhEnumPlug *)(listPlugData->Items[i]);
	delete plug;
    plug = 0;
	}
listPlugData->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::itemOpenClick(TObject *Sender)
{
//
if (imgOpenDialog->Execute())
   {
   if (ienView->IO->LoadFromFile(imgOpenDialog->FileName))
	  {
	  ienView->Proc->ClearAllUndo();
	  plugSetImage();
	  }
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::itemSaveClick(TObject *Sender)
{
srcImage = ienView->IEBitmap;
if (srcImage->Width < 2 || srcImage->Height < 2)
   return;
if (imgSaveDialog->Execute())
   {
   ienView->IO->SaveToFile(imgSaveDialog->FileName);
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::btnLoadListClick(TObject *Sender)
{
//
statusBar->Panels->Items[1]->Text = "Enumerating filers, please wait...";
statusBar->Refresh();
listCategory->Clear();
plugDeleteDataList();
pspiSetPath(editDir->Text.Trim().c_str());
pspiPlugInEnumerate(TNhelper::nhEnumerate, chkScanSubDirs->Checked);
if (listCategory->Count > 0)
   {
   plugCategoryBox->Clear();
   plugCategoryBox->Items->Assign(listCategory);
   plugCategoryBox->ItemIndex = 0;
   plugCategoryBoxClick(this);
   }
else
   {
   plugCategoryBox->Clear();
   plugNameBox->Clear();
   }
statusBar->Panels->Items[1]->Text = "";
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugCategoryBoxClick(TObject *Sender)
{
// selected item in category
if (plugCategoryBox->ItemIndex < 0)
   return;
// let's fill name list
AnsiString category = plugCategoryBox->Items->Strings[plugCategoryBox->ItemIndex];
TNhEnumPlug *plug;
plugNameBox->Clear();
for (int i = 0; i < listPlugData->Count; i++)
	{
	plug = (TNhEnumPlug *)(listPlugData->Items[i]);
	if (plug->category == category)
	   plugNameBox->Items->Add(plug->name);
	}
plugNameBox->ItemIndex = 0;
plugNameBoxClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::plugNameBoxClick(TObject *Sender)
{
plugSelected = L"";
if (plugNameBox->ItemIndex < 0)
   return;
AnsiString category = plugCategoryBox->Items->Strings[plugCategoryBox->ItemIndex];
AnsiString name = plugNameBox->Items->Strings[plugNameBox->ItemIndex];
TNhEnumPlug *plug;
for (int i = 0; i < listPlugData->Count; i++)
	{
	plug = (TNhEnumPlug *)(listPlugData->Items[i]);
	if (plug->category == category && plug->name == name)
	   {
	   plugSelected = plug->fullpath;
       break;
	   }
	}
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::itemUndoClick(TObject *Sender)
{
srcImage = ienView->IEBitmap;
if (srcImage->Width < 2 || srcImage->Height < 2)
   return;
if (ienView->Proc->CanUndo)
   {
   ienView->Proc->Undo(true);
   plugSetImage();  // set plugin image - don't forget!!
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::itemRedoClick(TObject *Sender)
{
srcImage = ienView->IEBitmap;
if (srcImage->Width < 2 || srcImage->Height < 2)
   return;
if (ienView->Proc->CanRedo)
   {
   ienView->Proc->Redo(true);
   plugSetImage();  // set plugin image - don't forget!!
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::itemMouseInteractClick(TObject *Sender)
{
TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
switch (item->Tag)
   {
   case 0:  // pan image
		ienView->MouseInteract = TIEMouseInteract()<<miScroll;
		break;
   case 1:  // zoom selection
		ienView->MouseInteract = TIEMouseInteract()<<miSelectZoom;
		break;
   case 2:  // select lasso
		ienView->MouseInteract = TIEMouseInteract()<<miSelectLasso;
		break;
   case 3:  // select polygon
		ienView->MouseInteract = TIEMouseInteract()<<miSelectPolygon;
		break;
   case 4:  // select rectangle
		ienView->MouseInteract = TIEMouseInteract()<<miSelect;
		break;
   case 5:  // select ellipse
		ienView->MouseInteract = TIEMouseInteract()<<miSelectCircle;
		break;
   case 6: // select magic wand
		ienView->MouseInteract = TIEMouseInteract()<<miSelectMagicWand;
		break;
   case 7: // clear selection
		ienView->Deselect();
		break;
   default:
		break;
   }
item->Checked = (item->Tag != 7);
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::itemResetZoomClick(TObject *Sender)
{
ienView->Zoom = 100.0;
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::itemFitToScreenClick(TObject *Sender)
{
ienView->Fit(true);
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::editWandTolValueChange(TObject *Sender)
{
ienView->MagicWandTolerance = editWandTol->IntValue;
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::chkSelMaxFilterValueChange(TObject *Sender)
{
ienView->MagicWandMaxFilter = chkSelMaxFilter->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::rgpWandModesClick(TObject *Sender)
{
ienView->MagicWandMode      = (TIEMagicWandMode)(rgpWandModes->ItemIndex);
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::btnAboutClick(TObject *Sender)
{
if (plugSelected.IsEmpty())
   {
   statusBar->Panels->Items[1]->Text = "Error, no filter selected.";
   return;
   }
statusBar->Panels->Items[1]->Text = "Loading filter...";
String filter = plugSelected;
int rc = pspiPlugInLoad(filter.c_str());
if (rc == 0)
   {
   statusBar->Panels->Items[1]->Text = "About filter...";
   statusBar->Refresh();
   void *FP = DisableTaskWindows((HWND)(this->Handle));  // so that plugin window stays on top like modal
   try {
	  rc = pspiPlugInAbout((HWND)(this->Handle));
   } catch (...) {
	  rc = -1;
   }
   EnableTaskWindows(FP);   // back to normal
   if (rc != 0)
	  {
	  statusBar->Panels->Items[1]->Text = "Error showing about window, rc =  " + IntToStr(rc);
	  }
   else
   	  statusBar->Panels->Items[1]->Text = "";
   }
else
   {
   statusBar->Panels->Items[1]->Text = "Error loading filter, rc = " + IntToStr(rc);
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::btnExecuteClick(TObject *Sender)
{
// execute filter
srcImage = ienView->IEBitmap;
if (srcImage->Width < 2 || srcImage->Height < 2)
   {
   statusBar->Panels->Items[1]->Text = "Error, image not loaded.";
   return;
   }
if (plugSelected.IsEmpty())
   {
   statusBar->Panels->Items[1]->Text = "Error, no filter selected.";
   return;
   }
TIERectangle sr;
plugDeleteMask();
plugSetROI();
// if there is selection, we'll use it as mask
if (ienView->Selected)
  {
  sr = ienView->SelectedRect;
  bool selres = (chkSelRestore->Checked && editSelFeather->IntValue > 0);
  if (selres)
	 {
	 ienView->SaveSelection();
	 ienView->MakeSelectionFeather(editSelFeather->IntValue);
	 }
  // let's set the mask
  srcMask = new TIEBitmap();
  srcMask->CopyFromTIEMask(ienView->SelectionMask);
  if (selres)
	 {
	 ienView->RestoreSelection();
	 ienView->MakeSelectionFeather(0);
	 }
  // for any case
  if (srcMask->Width != srcImage->Width || srcMask->Height != srcImage->Height)
	 plugDeleteMask();
  else
	 {
	 plugSetMask();
	 // set ROI if checked
	 if (chkFilterROI->Checked)
		plugSetROI(sr.y, sr.x, sr.height + sr.y, sr.width + sr.x);
	 }
  }
statusBar->Panels->Items[1]->Text = "Loading filter...";
progressPlug->Position = 0;
String filter = plugSelected;
int rc = pspiPlugInLoad(filter.c_str());
if (rc == 0)
   {
   statusBar->Panels->Items[1]->Text = "Executing filter...";
   statusBar->Refresh();
   ienView->LockUpdate();
   void *FP = DisableTaskWindows((HWND)(this->Handle));  // so that plugin window stays on top like modal
   // *********************************
   // temporary disable dialog styling:
   // if you don't use VCL styling or you don't have problems
   // with unwanted plugin windows styling, comment the line below
   // *********************************
   TStyleManager::SystemHooks = TStyleManager::SystemHooks >>  TStyleManager::shDialogs;
   try {
	  ienView->Proc->SaveUndo();
	  rc = pspiPlugInExecute((HWND)(this->Handle));
   } catch (...) {
	  rc = -1;
   }
   // *********************************
   // restore dialogs styling
   // *********************************
   TStyleManager::SystemHooks = TStyleManager::SystemHooks <<  TStyleManager::shDialogs;
   EnableTaskWindows(FP);   // back to normal
   ienView->UnLockUpdate();
   ienView->Update();
   progressPlug->Position = 0;
   if (rc != 0)
	  {
	  statusBar->Panels->Items[1]->Text = "Error executing filter, rc = " + IntToStr(rc);
	  }
   else
	  statusBar->Panels->Items[1]->Text = "";
   }
else
   {
   statusBar->Panels->Items[1]->Text = "Error loading filter, rc = " + IntToStr(rc);
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::btnDirClick(TObject *Sender)
{
WideString iDir = "";
String oDir = editDir->Text;
const String cap = "";
TSelectDirExtOpts options = TSelectDirExtOpts()<<TSelectDirExtOpt::sdNewUI<<TSelectDirExtOpt::sdShowEdit;
if (SelectDirectory(cap, iDir, oDir, options, this))
   {
   editDir->Text = oDir;
   editDir->Refresh();
   }
btnLoadListClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TfrmNiGulp::btnDonateClick(TObject *Sender)
{
TfrmNoPress *noPress = new TfrmNoPress(this);
noPress->ShowModal();
delete noPress;
}
//---------------------------------------------------------------------------


