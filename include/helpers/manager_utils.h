
#ifndef MANAGER_UTILS_H_INCLUDED

#define MANAGER_UTILS_H_INCLUDED


#include <iostream>
#include "UIAutomation.h"

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