//---------------------------------------------------------------------------
#pragma hdrstop
#include "TNhelper.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TNhEnumPlug TNhelper::nhPlugData = TNhEnumPlug();
TPlugEnumNotify TNhelper::nhPlugEnumNotify = 0;
TPlugProgressNotify TNhelper::nhPlugProgressNotify = 0;
TPlugColorPickerNotify TNhelper::nhPlugColorPickerNotify = 0;
//---------------------------------------------------------------------------
void TNhelper::nhEnumerate(const char *category, const char *name, const char *entrypoint, const wchar_t *fullpath)
{
	nhPlugData.category   = AnsiString(category);
	nhPlugData.name       = AnsiString(name);
	nhPlugData.entrypoint = AnsiString(entrypoint);
	nhPlugData.fullpath   = String(fullpath);
	if (nhPlugEnumNotify)
		nhPlugEnumNotify(&nhPlugData);
}
//---------------------------------------------------------------------------
void TNhelper::nhProgress(unsigned int done, unsigned int total)
{
   if (nhPlugProgressNotify)
		nhPlugProgressNotify(done, total);
}
//---------------------------------------------------------------------------
bool TNhelper::nhColorPicker(unsigned int &pickedColor)
{
   bool lRet = false;
   if (nhPlugColorPickerNotify)
	   lRet = nhPlugColorPickerNotify(pickedColor);
   return lRet;
}
