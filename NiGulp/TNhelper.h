//---------------------------------------------------------------------------
#ifndef TNhelperH
#define TNhelperH
#include <System.Classes.hpp>
struct TNhEnumPlug
	{
		AnsiString category;
		AnsiString name;
		AnsiString entrypoint;
		String fullpath;
		TNhEnumPlug()
		{
			category = "";
			name = "";
			entrypoint = "";
			fullpath = L"";
		}
	};
typedef void __fastcall (__closure *TPlugEnumNotify)(TNhEnumPlug *);
typedef void __fastcall (__closure *TPlugProgressNotify)(unsigned int, unsigned int);
typedef bool __fastcall (__closure *TPlugColorPickerNotify)(unsigned int &);
//---------------------------------------------------------------------------
class TNhelper
{
	private:
		static TNhEnumPlug nhPlugData;
	public:
		static TPlugEnumNotify nhPlugEnumNotify;
		static TPlugProgressNotify nhPlugProgressNotify;
        static TPlugColorPickerNotify nhPlugColorPickerNotify;
		static void nhEnumerate(const char *, const char *, const char *, const wchar_t *);
		static void nhProgress(unsigned int done, unsigned int total);
		static bool nhColorPicker(unsigned int &);
};
//---------------------------------------------------------------------------
#endif
