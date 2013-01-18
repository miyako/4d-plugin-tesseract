#include "4DPluginAPI.h"
#include "4DPlugin.h"

#include "functions_tocr.h"

#define THIS_BUNDLE_ID @"com.4D.4DPlugin.miyako.Tesseract"
#define THIS_BUNDLE_NAME L"Tesseract.4DX"

using namespace std;

C_TEXT langFolderPath;
unsigned int timeout = 15;

#pragma mark -

void OCRSetLangFolderPathAsEnvironmentVariable(C_TEXT &path)
{
#if VERSIONMAC
	NSString *posixPath = path.copyPath();
	CUTF8String u = CUTF8String((const uint8_t *)[posixPath UTF8String]);
	[posixPath release];
	//remove the folder separator
	if(u.size()) if(u.at(u.size() - 1) == ':') u.resize(u.size() - 1);		
	setenv("TESSDATA_PREFIX",(const char *)u.c_str(), 1);	
#else	
	CUTF16String u;
	path.copyUTF16String(&u);
	//remove the folder separator
	if(u.size()) if(u.at(u.size() - 1) == L'\\') u.resize(u.size() - 1);	
	_wputenv_s(L"TESSDATA_PREFIX", (const wchar_t *)u.c_str());
#endif
}

void OCRAddFolderSeparatorIfMissing(C_TEXT &path)
{
	CUTF8String u;
	path.copyUTF8String(&u);
	if(u.size())
	{
#if VERSIONMAC	
		if(u.at(u.size() - 1) != ':')
			u.append((const uint8_t *)":");	
#else	
		if(u.at(u.size() - 1) != '\\')
			u.append((const uint8_t *)"\\");
#endif	
	}
	path.setUTF8String(&u);
}
 
void OCRGetPluginResourcesFolderPath(C_TEXT &path)
{
#if VERSIONMAC	
	NSBundle *thisBundle = [NSBundle bundleWithIdentifier:THIS_BUNDLE_ID];
	if(thisBundle)
	{
		NSString *resourcePath = [thisBundle resourcePath];	
		if(resourcePath) 
		{
			//convert to hfs
			path.setPath(resourcePath);
			resourcePath = path.copyUTF16String();
			path.setUTF16String(resourcePath);
		}
	}
#else
	wchar_t	fDrive[_MAX_DRIVE], fDir[_MAX_DIR], fName[_MAX_FNAME], fExt[_MAX_EXT];
	wchar_t thisPath[_MAX_PATH] = {0};
	
	HMODULE hplugin = GetModuleHandleW(THIS_BUNDLE_NAME);
	
	GetModuleFileNameW(hplugin, thisPath, _MAX_PATH);
	
	_wsplitpath_s(thisPath, fDrive, fDir, fName, fExt);	
	wstring windowsPath = fDrive;
	windowsPath+= fDir;
	
	//remove delimiter to go one level up the hierarchy
	if(windowsPath.at(windowsPath.size() - 1) == L'\\') 
		windowsPath = windowsPath.substr(0, windowsPath.size() - 1);
	
	_wsplitpath_s(windowsPath.c_str(), fDrive, fDir, fName, fExt);
	wstring resourcesPath = fDrive;
	resourcesPath+= fDir;
	resourcesPath+= L"Resources";
	
	path.setUTF16String((PA_Unichar *)resourcesPath.c_str(), resourcesPath.size());	
#endif	
}

void OCRGetLangFolderPathDefault(C_TEXT &path)
{
	OCRGetPluginResourcesFolderPath(path);
	OCRAddFolderSeparatorIfMissing(path);
}

void OCRClearLangFolderPath()
{
	langFolderPath.setUTF8String((const uint8_t *)"", 0);
}
 
void OCRCheck()
{
	if(!langFolderPath.getUTF16Length())
		OCRGetLangFolderPathDefault(langFolderPath);
}

void OCRSetLangFolder(C_TEXT &path)
{
	langFolderPath.setUTF16String(path.getUTF16StringPtr(), path.getUTF16Length());
}

void OCRGetLangFolder(C_TEXT &path)
{
	OCRCheck();
	
	path.setUTF16String(langFolderPath.getUTF16StringPtr(), langFolderPath.getUTF16Length());
}

void OCRGetPathString(C_TEXT &path, string &pathString)
{
#if VERSIONMAC	
	NSString *posixPath = path.copyPath();
	pathString = string((const char *)[posixPath UTF8String]);
	[posixPath release];	
#else
	setlocale(LC_CTYPE, "");	
	size_t size = (path.getUTF16Length() * MB_CUR_MAX) + 1;	
	vector<char> buf(size);
	wcstombs((char *)&buf[0], (const wchar_t *)path.getUTF16StringPtr(), size);
	pathString = string((const char *)&buf[0]);	
#endif	
}

void OCRGetLangFolderPathString(string &pathString)
{
	OCRSetLangFolderPathAsEnvironmentVariable(langFolderPath);
	OCRGetPathString(langFolderPath, pathString);
}

#pragma mark -

// ----------------------------------- tesseract ----------------------------------

void OCR_Read_image_text(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	C_TEXT Param2;
	C_TEXT Param3;
	C_LONGINT returnValue;
	
	Param1.fromParamAtIndex(pParams, 1);
	Param3.fromParamAtIndex(pParams, 3);
	
	OCRCheck();

	string imageFilePath, datapath;
	
	OCRGetPathString(Param1, imageFilePath);
	OCRGetLangFolderPathString(datapath);
	
	const char *lang = NULL;
	CUTF8String language;
	
	if(Param3.getUTF16Length()) 
	{
		Param3.copyUTF8String(&language);
		lang = (const char *)language.c_str();
	}
		
	tesseract::TessBaseAPI api;

	api.SetPageSegMode(tesseract::PSM_AUTO_OSD);	

	PIX *pixs = 0;
	
	pixs = pixRead(imageFilePath.c_str());

	//The datapath must be the name of the data directory (no ending /)
	//The language is (usually) an ISO 639-3 string or NULL will default to eng.
	//return: 0 on success and -1 on initialization failure.
	
	if(!api.Init(datapath.c_str(), lang))
	{
		FILE* fin = fopen(imageFilePath.c_str(), "rb");
		
		if(fin)
		{
			fclose(fin);
	
			if(pixs)
			{
				api.SetImage(pixs);
				char* outText = api.GetUTF8Text();
				
				if (outText)
				{
					Param2.setUTF8String((const uint8_t *)outText, strlen(outText));
					outText = 0;
				}else{returnValue.setIntValue(-4);}//Error during processing.				
				pixDestroy(&pixs);
			}else{returnValue.setIntValue(-3);}//Unsupported image type.
		}else{returnValue.setIntValue(-2);}//Can't open input file.
	}else{returnValue.setIntValue(-1);}//Error opening data file.
	
		api.Clear();
		api.End();

	Param2.toParamAtIndex(pParams, 2);
	returnValue.setReturn(pResult);
}

void OCR_Get_language_folder(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT returnValue;
	
	OCRGetLangFolder(returnValue);
	
	returnValue.setReturn(pResult);
}

void OCR_Set_language_folder(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	C_LONGINT returnValue;
	
	Param1.fromParamAtIndex(pParams, 1);
	
	if(Param1.getUTF16Length())
	{
		//store the current value
		C_TEXT originalPath;
		OCRGetLangFolder(originalPath);
		
		string datapath;
		OCRSetLangFolder(Param1);
		OCRGetLangFolderPathString(datapath);
		
		//test if the api can be initialized
		tesseract::TessBaseAPI api;
		if(!api.Init(datapath.c_str(), NULL))
		{
			api.End();
			returnValue.setIntValue(1);	
		}else{
			OCRSetLangFolder(originalPath);
		}
	}else{
		//revert to default
		OCRClearLangFolderPath();
		returnValue.setIntValue(1);
	}
	
	returnValue.setReturn(pResult);
}

void OCR_Get_timeout(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;
	
	returnValue.setIntValue(timeout);
	returnValue.setReturn(pResult);
}

void OCR_SET_TIMEOUT(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;
	
	Param1.fromParamAtIndex(pParams, 1);
	timeout = (unsigned int)(Param1.getIntValue());
}