
#ifndef MANAGER_UTILS_H_INCLUDED

#define MANAGER_UTILS_H_INCLUDED


#include <iostream>
#include "UIAutomation.h"
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <winreg.h>
#include "..\..\external\json\json.hpp"
#include <iostream>
#include <string>
#include <locale>
#include <codecvt> // Required for std::wstring_convert and std::codecvt_utf8
#include <Windows.h>
#include <UIAutomation.h>
#include <comutil.h>
#include <string>
#include <iostream>
#include <windows.h>
#include "..\..\external\opencv2\opencv.hpp"
#include <string>

using namespace cv;

using json = nlohmann::json;

// Convert VARIANT to std::string
inline std::string variantToString__NodeElement(const VARIANT &var) {
    switch (var.vt) {
        case VT_BSTR: {
            _bstr_t b(var.bstrVal);
            return std::string((const char*)b);
        }
        case VT_I4:
            return std::to_string(var.lVal);

        case VT_UI4:
            return std::to_string(var.ulVal);

        case VT_R8:
            return std::to_string(var.dblVal);

        case VT_BOOL:
            return var.boolVal ? "true" : "false";

        case VT_EMPTY:
        case VT_NULL:
            return "";

        default:
            return "<unsupported VARIANT type>";
    }
}

// Extract value of an element and return as JSON
json extractValueToJson(IUIAutomationElement* elem) ;
// Function to convert wstring to string (UTF-8)
std::string WstringToUtf8String(const std::wstring& wstr);
// Helper to read registry string values
static std::wstring QueryRegValue(HKEY hKey, const wchar_t* valueName) ;
json GetInstalledApplicationsJson() ;
std::string getElementType(int controlTypeId);

std::string variantToString(const VARIANT& v);
bool IsModalDialog(IUIAutomationElement* elem);
// -----------------------------
// Convert HBITMAP → cv::Mat
// -----------------------------
cv::Mat HBitmapToMat(HBITMAP hBmp);
// -----------------------------
// Take full screen screenshot
// -----------------------------
cv::Mat takeScreenshot();
cv::Mat takeScreenshot_full();

// -----------------------------
// Draw grid on cv::Mat
// -----------------------------
void drawGrid(cv::Mat &img);

#endif