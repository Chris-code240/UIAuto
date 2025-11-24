
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

using json = nlohmann::json;

// Convert VARIANT to std::string
static std::string variantToString__NodeElement(const VARIANT &var) {
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
json extractValueToJson(IUIAutomationElement* elem) {

    if (!elem)
        return json{ {"error", "null element"} };

    VARIANT retVal;
    VariantInit(&retVal);

    HRESULT hr = elem->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &retVal);

    if (FAILED(hr)) {
        return json{ {"error", "GetCurrentPropertyValue failed"} };
    }

    // Convert VARIANT → string
    std::string value = variantToString__NodeElement(retVal);

    VariantClear(&retVal);

    // Put into JSON
    json result = {
        {"value", value}
    };

    return result;
}


// Function to convert wstring to string (UTF-8)
std::string WstringToUtf8String(const std::wstring& wstr) {
    // Setup converter for wstring (UTF-16) to string (UTF-8)
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    try {
        return converter.to_bytes(wstr);
    } catch (const std::range_error& e) {
        // Handle conversion error, which usually happens if invalid characters are encountered
        std::cerr << "Conversion error: " << e.what() << std::endl;
        return {}; // Return empty string or handle as appropriate
    }
}



using json = nlohmann::json;

// Helper to read registry string values
static std::wstring QueryRegValue(HKEY hKey, const wchar_t* valueName) {
    DWORD type = 0;
    DWORD size = 0;

    if (RegQueryValueExW(hKey, valueName, nullptr, &type, nullptr, &size) != ERROR_SUCCESS)
        return L"";

    if (type == REG_SZ || type == REG_EXPAND_SZ) {
        std::wstring result;
        result.resize(size / sizeof(wchar_t));
        if (RegQueryValueExW(hKey, valueName, nullptr, nullptr, (LPBYTE)&result[0], &size) == ERROR_SUCCESS) {
            if (!result.empty() && result.back() == L'\0')
                result.pop_back();
            return result;
        }
    }
    return L"";
}

json GetInstalledApplicationsJson() {
    CoInitialize(NULL);
    json apps = json::object();

    std::vector<std::wstring> uninstall_keys = {
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    };

    for (const auto& subKeyPath : uninstall_keys) {
        HKEY hKey = nullptr;

        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subKeyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD index = 0;
            wchar_t subKeyName[256];
            DWORD nameSize = 256;

            while (RegEnumKeyExW(hKey, index, subKeyName, &nameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                HKEY hAppKey = nullptr;

                if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &hAppKey) == ERROR_SUCCESS) {
                    std::wstring displayName     = QueryRegValue(hAppKey, L"DisplayName");
                    std::wstring installLocation = QueryRegValue(hAppKey, L"InstallLocation");
                    std::wstring displayVersion  = QueryRegValue(hAppKey, L"DisplayVersion");
                    std::wstring systemComponent = QueryRegValue(hAppKey, L"SystemComponent");
                    std::wstring parentKeyName   = QueryRegValue(hAppKey, L"ParentKeyName");

                    // Only add real software entries
                    if (!displayName.empty() && 
                        systemComponent != L"1" && 
                        parentKeyName.empty()) 
                    {
                        apps[WstringToUtf8String(displayName)] = {
                            { "path",   WstringToUtf8String(installLocation) },
                            { "version", WstringToUtf8String(displayVersion) }
                        };
                    }

                    RegCloseKey(hAppKey);
                }

                index++;
                nameSize = 256;
            }

            RegCloseKey(hKey);
        }
    }
    CoUninitialize();
    return apps;
}


std::string getElementType(int controlTypeId)
{
    switch (controlTypeId){

        case UIA_ButtonControlTypeId:            return "Button";
        case UIA_EditControlTypeId:              return "Edit";
        case UIA_TextControlTypeId:              return "Text";
        case UIA_CheckBoxControlTypeId:          return "CheckBox";
        case UIA_ComboBoxControlTypeId:          return "ComboBox";
        case UIA_ListControlTypeId:              return "List";
        case UIA_ListItemControlTypeId:          return "ListItem";
        case UIA_MenuControlTypeId:              return "Menu";
        case UIA_MenuItemControlTypeId:          return "MenuItem";
        case UIA_PaneControlTypeId:              return "Pane";
        case UIA_WindowControlTypeId:            return "Window";
        case UIA_HyperlinkControlTypeId:         return "Hyperlink";
        case UIA_TreeControlTypeId:              return "Tree";
        case UIA_TreeItemControlTypeId:          return "TreeItem";
        case UIA_TabControlTypeId:               return "Tab";
        case UIA_TabItemControlTypeId:           return "TabItem";
        case UIA_RadioButtonControlTypeId:       return "RadioButton";
        case UIA_GroupControlTypeId:             return "Group";
        case UIA_DataItemControlTypeId:          return "DataItem";
        case UIA_TableControlTypeId:             return "Table";
        case UIA_CustomControlTypeId:            return "Custom";
        case UIA_ImageControlTypeId:             return "Image";
        case UIA_SliderControlTypeId:            return "Slider";
        case UIA_SpinnerControlTypeId:           return "Spinner";
        case UIA_StatusBarControlTypeId:         return "StatusBar";
        case UIA_ToolBarControlTypeId:           return "ToolBar";
        case UIA_ToolTipControlTypeId:           return "ToolTip";
        default:                                 return "Unknown";
    }
}


std::string variantToString(const VARIANT& v)
{
    if (v.vt != VT_BSTR || v.bstrVal == nullptr)
        return "";

    int len = SysStringLen(v.bstrVal);
    if (len == 0)
        return "";

    // Convert UTF-16 BSTR → UTF-8 std::string
    int needed = WideCharToMultiByte(CP_UTF8, 0, v.bstrVal, len, nullptr, 0, nullptr, nullptr);
    if (needed <= 0)
        return "";

    std::string out(needed, '\0');
    WideCharToMultiByte(CP_UTF8, 0, v.bstrVal, len, &out[0], needed, nullptr, nullptr);

    return out;
}

bool IsModalDialog(IUIAutomationElement* elem) {
    if (!elem) return false;
// The window’s parent
    IUIAutomationElement* parent = nullptr;
    elem->GetCachedParent(&parent);   // or GetParent()

    // Check parent's WindowInteractionState
    IUIAutomationWindowPattern* pat = nullptr;
    parent->GetCurrentPatternAs(UIA_WindowPatternId,
                                IID_PPV_ARGS(&pat));

    WindowInteractionState st;
    pat->get_CurrentWindowInteractionState(&st);
    if (parent) parent->Release();
    return (st != WindowInteractionState_BlockedByModalWindow && st == WindowInteractionState_ReadyForUserInteraction);
}


#endif