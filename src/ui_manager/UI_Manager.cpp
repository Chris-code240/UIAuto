#include "../../include/ui_manager/UI_Manager.h"
#include "../../include/helpers/manager_utils.h"
using namespace UI_Automation;


UIManager::UIManager(){

    HRESULT hr;
    this->IUIA = this->createUIAObject();
    if (! this->IUIA){
        throw std::runtime_error("IUIAutomation object could not be created.");
        abort();
    }
      hr = this->IUIA->GetRootElement(&this->pRoot);
    if (FAILED(hr)){
        throw std::runtime_error("UIManager could not grab Root Element");
        abort();
    }
    WindowEventHandler* handler = new WindowEventHandler(this);
    
    hr =  this->IUIA->AddAutomationEventHandler( UIA_Window_WindowOpenedEventId,this->pRoot, TreeScope_Subtree, NULL,handler );

    if (FAILED(hr) || FAILED(this->IUIA->AddAutomationEventHandler( UIA_Window_WindowClosedEventId,this->pRoot, TreeScope_Subtree, NULL,handler ))){
        throw std::runtime_error("Coudl not register handler");
        abort();
    }

    this->allApps += GetInstalledAppsFromRegistry(HKEY_LOCAL_MACHINE);
    this->allApps += GetInstalledAppsFromRegistry(HKEY_CURRENT_USER);

    StructureChangedEventHandler* sch = new StructureChangedEventHandler(this);
    IUIA->AddStructureChangedEventHandler(  pRoot,  TreeScope_Subtree,  NULL, sch);

}


UIManager::~UIManager(){
    std::cout<<computeDiffs().dump(4);
    if (this->IUIA != nullptr) this->IUIA->Release();
    if (this->pRoot != nullptr)  this->pRoot->Release();
    CoUninitialize();
}

bool UIManager::IsSecureDesktopActive() {
    // Check if a UAC (User Account Control) modal has popped up.
    HDESK hDesk = OpenInputDesktop(0, FALSE, GENERIC_READ);
    if (!hDesk) return false;

    WCHAR name[256];
    DWORD needed = 0;
    bool isSecure = false;

    if (GetUserObjectInformationW(hDesk, UOI_NAME, name, sizeof(name), &needed)) {
        isSecure = (_wcsicmp(name, L"Winlogon") == 0);
    }

    CloseDesktop(hDesk);
    return isSecure;
}

IUIAutomationElement * UIManager::getDesktop(){
    HRESULT hr;
    hr = this->IUIA->GetRootElement(&this->pRoot);
    if (FAILED(hr)) return nullptr;
    return this->pRoot;
}

IUIAutomationElement * UIManager::getRoot(){
    HRESULT hr;
    hr = this->IUIA->GetRootElement(&this->pRoot);
    if (FAILED(hr)) return nullptr;
    return this->pRoot;
}

json UIManager::getDesktopSnapshot() {
    json result = json::array();

    HRESULT hr;
    IUIAutomationCondition* pCond = nullptr;
    IUIAutomationElementArray* subElements = nullptr;

    hr = this->IUIA->CreateTrueCondition(&pCond);
    if (FAILED(hr)) throw std::runtime_error("CreateTrueCondition failed");

    hr = this->getDesktop()->FindAll(TreeScope_Children, pCond, &subElements);
    pCond->Release();

    if (FAILED(hr)) throw std::runtime_error("FindAll failed");

    int length = 0;
    if (FAILED(subElements->get_Length(&length)))
        throw std::runtime_error("Could not get Desktop Elements Length");

    for (int i = 0; i < length; i++) {
        IUIAutomationElement* element = nullptr;
        if (FAILED(subElements->GetElement(i, &element)))
            throw std::runtime_error("GetElement failed");

        VARIANT varName;
        VariantInit(&varName);

        RECT boundingBox{};
        if (FAILED(element->GetCurrentPropertyValue(UIA_NamePropertyId, &varName)))
            throw std::runtime_error("GetCurrentPropertyValue failed");

        if (FAILED(element->get_CurrentBoundingRectangle(&boundingBox)))
            throw std::runtime_error("BoundingRectangle failed");

        std::string name =
            (varName.vt == VT_BSTR && varName.bstrVal != nullptr)
            ? std::string(_bstr_t(varName.bstrVal))
            : "(no name)";

        VariantClear(&varName);

        json item = {
            {"name", name},
            {"boundingBox", {
                {"left", boundingBox.left},
                {"top", boundingBox.top},
                {"right", boundingBox.right},
                {"bottom", boundingBox.bottom}
            }}
        };

        result.push_back(item);

        if (element) element->Release();
    }

    subElements->Release();
    return result;
}

bool UIManager::createWindow(LPWSTR windowName){
 
    return false;


}

json UIManager::selectWindow(std::string windowName){
    if (windowName.empty()) return false;

    HRESULT hr;
    IUIAutomationElement * window = nullptr;
    IUIAutomationCondition * windowCond = nullptr;
    IUIAutomationWindowPattern* windowPattern = nullptr;
    
    // string -> wstring -> LPWSTR
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(windowName);
    LPWSTR name = wstr.data();

    hr = this->IUIA->CreatePropertyCondition( UIA_NamePropertyId, _variant_t(name), &windowCond);
    if (FAILED(hr)) json{{"success", false}};;

    hr = this->pRoot->FindFirst(TreeScope_Children, windowCond, &window);

    if (SUCCEEDED(hr)){
        VARIANT varHandle;
        hr = window->GetCurrentPropertyValue(UIA_NativeWindowHandlePropertyId, &varHandle);
        if (SUCCEEDED(hr) && varHandle.vt == VT_I4) {
            HWND hwnd = (HWND)(LONG_PTR)varHandle.lVal;
            SetForegroundWindow(hwnd);

            goto cleanup;
        }
    }
    cleanup:
        if (windowPattern) windowPattern->Release();
        if (window) window->Release();
        if (windowCond) windowCond->Release();
        return json{{"success", SUCCEEDED(hr)}};

}

bool UIManager::killWindow(std::wstring windowName){
    HRESULT hr;

    IUIAutomationElement* foundElement = nullptr;
    IUIAutomationCondition* cond = nullptr;
    IUIAutomationWindowPattern* windowPattern = nullptr;

    LPWSTR s = windowName.data();
    hr = this->IUIA->CreatePropertyCondition(
        UIA_NamePropertyId,
        _variant_t(s),
        &cond
    );

    if (FAILED(hr) || !cond)
        goto cleanup;

    hr = this->pRoot->FindFirst(TreeScope_Children, cond, &foundElement);

    if (FAILED(hr) || !foundElement)
        goto cleanup;

    hr = foundElement->GetCurrentPatternAs(
        UIA_WindowPatternId,
        IID_IUIAutomationWindowPattern,
        (void**)&windowPattern
    );

    if (FAILED(hr) || !windowPattern)
        goto cleanup;

    hr = windowPattern->Close();
    
    
    if (FAILED(hr))
        goto cleanup;

    cleanup:
        if (windowPattern) windowPattern->Release();
        if (foundElement) foundElement->Release();
        if (cond) cond->Release();

        return (SUCCEEDED(hr));
}

json UIManager::leftClick(int x, int y){
    json data = json::array();

    /*
    Some clicks result in pop-ups (including ddialogs) and drop downs. 

    Explore these:
        UIA_MenuControlTypeId
        UIA_PaneControlTypeId
        UIA_ListControlTypeId
        UIA_WindowControlTypeId
    */



    IUIAutomationElement * targetElement = nullptr;

    IUIAutomationCondition * dialogsCond = nullptr;
    IUIAutomationCondition * windowsBeforeClickCond = nullptr;
    IUIAutomationElementArray * windowsBeforeClick = nullptr;
    IUIAutomationElementArray * openedDialogs = nullptr;

    INPUT inputs[2] = {};

    int numberOfDialogs = 0;
    if (!x && !y ) goto cleanup;
    HRESULT hr;

    SetCursorPos(x ,y );

    // prepare input and send mouse input
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(2, inputs, sizeof(INPUT));
    Sleep(200);
    // get all opened dialogs
    this->IUIA->CreatePropertyCondition(UIA_ControlTypePropertyId, _variant_t(UIA_WindowControlTypeId), &windowsBeforeClickCond);
    hr = this->pRoot->FindAll(TreeScope_Subtree, dialogsCond, &windowsBeforeClick);
    if (FAILED(hr)) goto cleanup;
    Sleep(2000);

    
   
    goto cleanup;

    cleanup:

        if (targetElement) targetElement->Release();
        if (openedDialogs) openedDialogs->Release();
        if (dialogsCond) dialogsCond->Release();
        return data;
}

json UIManager::rightClick(IUIAutomationElement * targetElement, int x , int y){
    HRESULT hr;
    RECT rectVal;
    
    json data = json::array();
    if (!targetElement && ! x && !y) return data;

    if (!targetElement){
        POINT p;
        p.x = x;
        p.y = y;
        hr = this->IUIA->ElementFromPoint(p, &targetElement);
        if (FAILED(hr))  return data;
    }
    BSTR elementName;
    targetElement->get_CurrentName(&elementName);
    targetElement->get_CurrentBoundingRectangle(&rectVal);
    SetCursorPos((int)(rectVal.left + rectVal.right)/ 2, (int)(rectVal.top + rectVal.bottom) /2);

    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;

    SendInput(2, inputs, sizeof(INPUT));

    Sleep(200); 

    // Find the menu container
    IUIAutomationCondition* menuCond = nullptr;
    hr = this->IUIA->CreatePropertyCondition( UIA_ControlTypePropertyId, _variant_t(UIA_MenuControlTypeId),  &menuCond );

    IUIAutomationElement* menuRoot = nullptr;
    hr = this->pRoot->FindFirst(TreeScope_Subtree, menuCond, &menuRoot);
    menuCond->Release();

    if (FAILED(hr) || !menuRoot) {
        return data;
    }

    // Now find menu items INSIDE that menu
    IUIAutomationCondition* itemCond = nullptr;
    hr = this->IUIA->CreatePropertyCondition( UIA_ControlTypePropertyId, _variant_t(UIA_MenuItemControlTypeId),&itemCond);

    IUIAutomationElementArray* menuItems = nullptr;
    hr = menuRoot->FindAll(TreeScope_Subtree, itemCond, &menuItems);
    itemCond->Release();
    menuRoot->Release();

  
    if (FAILED(hr) || !menuItems) {
        return data;
    }

    int length;
    menuItems->get_Length(&length);


    while(length > 0){
        length -= 1;
        IUIAutomationElement * item = nullptr;
        menuItems->GetElement(length, &item);
        if (item){
            BSTR itemName;
            item->get_CurrentName(&itemName);
            RECT itemBoundingBox;
            item->get_CurrentBoundingRectangle(&itemBoundingBox);
            data.push_back({{"menuItemName", itemName},{"menuItemBoundingBox",{{"top", itemBoundingBox.top},{"right", itemBoundingBox.right},{"bottom", itemBoundingBox.bottom},{"left", itemBoundingBox.left} }} });
            item->Release();
        }
    }
    
    // if (menuCond != nullptr) menuCond->Release();
    if (menuItems) menuItems->Release();
    if (targetElement != nullptr) targetElement->Release();

       
    return data;
}

json UIManager::getWindowElements(std::string windowName, IUIAutomationElement * window){

    HRESULT hr;
    if(! window && windowName.empty()) throw std::runtime_error("WIndow or WIndowName must be provided");
    IUIAutomationCondition *foundWindowCond = nullptr;
    IUIAutomationElementArray *windowElements = nullptr;
    IUIAutomationCondition *windowElementsCond  = nullptr;
    IUIAutomationElement *foundWindow = nullptr;
    int length;
    json data = json::array();

    //create condition to find window by name
    hr = this->IUIA->CreatePropertyCondition(UIA_NamePropertyId, _variant_t(windowName.c_str()), &foundWindowCond);

    if (SUCCEEDED(hr) && SUCCEEDED(this->pRoot->FindFirst(TreeScope_Children, foundWindowCond, &foundWindow))){

        // Get elements
        if (SUCCEEDED(this->IUIA->CreateTrueCondition(&windowElementsCond)) && SUCCEEDED(foundWindow->FindAll(TreeScope_Descendants,windowElementsCond, &windowElements))){
            
            int length = 0;
            windowElements->get_Length(&length);
            for(int i = 0 ; i < length; i++){
                IUIAutomationElement * ele = nullptr;
                BSTR varName;
                VARIANT var;
                VARIANT varCtrl;
                RECT elementRect;
                BOOL gotClickablePoint;
                POINT pt;
                hr = windowElements->GetElement(i, &ele);
                if (SUCCEEDED(hr)){
                    if (SUCCEEDED(ele->GetCurrentPropertyValue(UIA_NamePropertyId, &var)) && 
                    SUCCEEDED(ele->GetCurrentPropertyValue(UIA_ControlTypePropertyId, &varCtrl)) && 
                    SUCCEEDED(ele->get_CurrentBoundingRectangle(&elementRect)) && SUCCEEDED(ele->GetClickablePoint(&pt,&gotClickablePoint)))   {
                        int len = SysStringLen(var.bstrVal);
                        char* buffer = new char[len + 1];
                        // Convert wide char BSTR to multibyte
                        wcstombs(buffer, var.bstrVal, len);
                        buffer[len] = '\0';  // Null-terminate
                std::string elementType =
                    varCtrl.intVal == UIA_ButtonControlTypeId ? "buttonElement" :
                    varCtrl.intVal == UIA_EditControlTypeId ? "inputElement" :
                    varCtrl.intVal == UIA_TextControlTypeId ? "textElement" :
                    varCtrl.intVal == UIA_ProgressBarControlTypeId ? "progressBarElement" :
                    varCtrl.intVal == UIA_ImageControlTypeId ? "imageElement" :
                    varCtrl.intVal == UIA_TableControlTypeId ? "tableElement" :
                    varCtrl.intVal == UIA_TabControlTypeId ? "tabElement" :
                    varCtrl.intVal == UIA_CheckBoxControlTypeId ? "checkboxElement" :
                    varCtrl.intVal == UIA_ComboBoxControlTypeId ? "comboboxElement" :
                    varCtrl.intVal == UIA_ListControlTypeId ? "listElement" :
                    varCtrl.intVal == UIA_ListItemControlTypeId ? "listItemElement" :
                    varCtrl.intVal == UIA_MenuControlTypeId ? "menuElement" :
                    varCtrl.intVal == UIA_MenuItemControlTypeId ? "menuItemElement" :
                    varCtrl.intVal == UIA_HyperlinkControlTypeId ? "hyperlinkElement" :
                    varCtrl.intVal == UIA_PaneControlTypeId ? "paneElement" :
                    varCtrl.intVal == UIA_WindowControlTypeId ? "windowElement" :
                    varCtrl.intVal == UIA_TreeControlTypeId ? "treeElement" :
                    varCtrl.intVal == UIA_TreeItemControlTypeId ? "treeItemElement" :
                    varCtrl.intVal == UIA_SliderControlTypeId ? "sliderElement" :
                    varCtrl.intVal == UIA_ScrollBarControlTypeId ? "scrollbarElement" :
                    varCtrl.intVal == UIA_DocumentControlTypeId ? "documentElement" :
                    varCtrl.intVal == UIA_GroupControlTypeId ? "groupElement" :
                    varCtrl.intVal == UIA_CustomControlTypeId ? "customElement" :
                    "unknownElement";
                        json t = {
                            {"elementName", buffer}, 
                            {"elementType",elementType}, 
                            {"boundingBox",
                                { 
                                    {"top", elementRect.top}, 
                                    {"right", elementRect.right}, 
                                    {"bootom", elementRect.bottom}, 
                                    {"left", elementRect.left}
                                }
                            }, 
                            {"clickablePoint", {
                                { "x", pt.x}, 
                                {"y", pt.y}
                            }
                            }
                        };
                        data.push_back(t);
                        delete[] buffer;
                    }
                }
            }
        }
        
        
    }
    if (foundWindowCond) foundWindowCond -> Release();
    if (windowElements) windowElements->Release();
    if (windowElementsCond) windowElementsCond->Release();
    if (foundWindow) foundWindow->Release();
    return data;
}

void UIManager::DrawRect(const RECT& rect, COLORREF color){
        HDC hdc = GetDC(NULL); // entire screen
        HPEN hPen = CreatePen(PS_SOLID, 2, color);
        HGDIOBJ oldPen = SelectObject(hdc, hPen);
        HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(hPen);
        ReleaseDC(NULL, hdc);
    }

json UIManager::GetInstalledAppsFromRegistry(HKEY rootKey) {
    json apps = json::array();

    const wchar_t* uninstallPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    HKEY hUninstall = nullptr;

    if (RegOpenKeyExW(rootKey, uninstallPath, 0, KEY_READ, &hUninstall) != ERROR_SUCCESS)
        return apps;

    DWORD index = 0;
    wchar_t subkeyName[256];
    DWORD subkeyLen = 256;

    while (RegEnumKeyExW(hUninstall, index++, subkeyName, &subkeyLen, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        HKEY hApp = nullptr;
        if (RegOpenKeyExW(hUninstall, subkeyName, 0, KEY_READ, &hApp) == ERROR_SUCCESS) {
            wchar_t displayName[256] = L"";
            wchar_t installLoc[512] = L"";
            wchar_t exePath[512] = L"";
            DWORD size;

            // App Name
            size = sizeof(displayName);
            RegQueryValueExW(hApp, L"DisplayName", nullptr, nullptr, (LPBYTE)displayName, &size);

            // Install Location (real folder of app)
            size = sizeof(installLoc);
            RegQueryValueExW(hApp, L"InstallLocation", nullptr, nullptr, (LPBYTE)installLoc, &size);

            // Uninstall string (fallback path)
            size = sizeof(exePath);
            RegQueryValueExW(hApp, L"UninstallString", nullptr, nullptr, (LPBYTE)exePath, &size);

            if (wcslen(displayName) > 0) {
                std::wstring name(displayName);
                std::wstring path;

                // Prefer valid InstallLocation if it exists
                if (wcslen(installLoc) > 0 && GetFileAttributesW(installLoc) != INVALID_FILE_ATTRIBUTES)
                    path = installLoc;
                else if (wcslen(exePath) > 0)
                    path = exePath;

                // Only add if the path exists or looks executable
                if (!path.empty()) {
                    apps.push_back({
                        {"name", std::string(name.begin(), name.end())},
                        {"path", std::string(path.begin(), path.end())}
                    });
                }
            }
            RegCloseKey(hApp);
        }
        subkeyLen = 256;
    }

    RegCloseKey(hUninstall);
    return apps;
}

bool UIManager::open_app(const std::string& appName) {
    auto apps = getAllApps(); // combines HKLM + HKCU
    for (auto& app : apps) {
        if (app["name"] == appName) {
            std::string path = app["path"];
            if (path.empty()) return false;
            ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
            return true;
        }
    }
    return false;
}

bool UIManager::dragAndDrop(int from_x, int from_y, int to_x, int to_y) {
    if (!from_x && !from_y && !to_x && !to_y)
        return false;

    // Move to starting point
    SetCursorPos(from_x, from_y);
    Sleep(50);

    // Press mouse button down
    INPUT mouseDown = {};
    mouseDown.type = INPUT_MOUSE;
    mouseDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &mouseDown, sizeof(INPUT));
    Sleep(50); // small human-like delay

    // Drag movement — smooth movement
    const int steps = 30; 
    for (int i = 0; i < steps; i++) {
        int x = from_x + (to_x - from_x) * i / steps;
        int y = from_y + (to_y - from_y) * i / steps;
        SetCursorPos(x, y);
        Sleep(5);
    }

    // Snap to final position
    SetCursorPos(to_x, to_y);
    Sleep(50);

    // Release mouse button
    INPUT mouseUp = {};
    mouseUp.type = INPUT_MOUSE;
    mouseUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &mouseUp, sizeof(INPUT));

    return true;
}

bool UIManager::minimizeWindow(std::string windowName){
    return true;
}

json UIManager::hover(int x, int y){

    json data = json::array();
    if(!x && ! y) return data;

    // Position mouse at given point
    SetCursorPos(x, y);
    Sleep(50);
    HRESULT hr;
    IUIAutomationElement * targetElement = nullptr;
    POINT p;
    p.x = x; p.y = y;
    hr = this->IUIA->ElementFromPoint(p,&targetElement);

    if(FAILED (hr)) return data;

    return data;
}

json UIManager::computeDiffs() {
    json events = json::array();

    // 1. Added elements
    for (auto& [id, el] : uiDom) {
        if (!uiDomPrev.count(id)) {
            events.push_back({
                {"event","element_added"},
                {"id", id},
                {"name", el.name},
                {"type", el.type}
            });
        }
    }

    // 2. Removed elements
    for (auto& [id, el] : uiDomPrev) {
        if (!uiDom.count(id)) {
            events.push_back({
                {"event","element_removed"},
                {"id", id}
            });
        }
    }

    // 3. Modified bounds / name changes
    for (auto& [id, el] : uiDom) {
        if (!uiDomPrev.count(id)) continue;

        const auto& old = uiDomPrev[id];

        if (memcmp(&old.bounds, &el.bounds, sizeof(RECT)) != 0) {
            events.push_back({
                {"event","bounds_changed"},
                {"id", id},
                {"old", {
                    {"left", old.bounds.left},
                    {"top", old.bounds.top},
                    {"right", old.bounds.right},
                    {"bottom", old.bounds.bottom},
                }},
                {"new", {
                    {"left", el.bounds.left},
                    {"top", el.bounds.top},
                    {"right", el.bounds.right},
                    {"bottom", el.bounds.bottom},
                }}
            });
        }

        if (old.name != el.name) {
            events.push_back({
                {"event","name_changed"},
                {"id", id},
                {"old", old.name},
                {"new", el.name}
            });
        }
    }

    return events;
}



void UIManager::captureWindow(IUIAutomationElement* win) {
    // Save previous UI state
    uiDomPrev = uiDom;
    uiDom.clear();

    IUIAutomationCondition* cond = nullptr;
    IUIA->CreateTrueCondition(&cond);

    IUIAutomationElementArray* elems = nullptr;
    win->FindAll(TreeScope_Subtree, cond, &elems);
    cond->Release();

    if (!elems) return;

    int length = 0;
    elems->get_Length(&length);

    for (int i = 0; i < length; i++) {
        IUIAutomationElement* e = nullptr;
        elems->GetElement(i, &e);
        if (!e) continue;

        ElementInfo info;

        // Generate stable ID
        info.id = makeStableId(e);

        // Name
        BSTR en = nullptr;
        e->get_CurrentName(&en);
        info.name = en ? std::string(_bstr_t(en)) : "(no name)";
        if (en) SysFreeString(en);

        // Type
        e->get_CurrentControlType(&info.type);

        // Bounds
        e->get_CurrentBoundingRectangle(&info.bounds);

        // Parent
        IUIAutomationElement* parent = nullptr;
        e->GetCachedParent(&parent);
        if (parent) {
            info.parent = makeStableId(parent);
            parent->Release();
        }

        uiDom[info.id] = info;

        e->Release();
    }

    elems->Release();

    // Now compute diffs
    auto diffs = computeDiffs();

    // (Optional) Signal the LLM or store diffs
    // if (!diffs.empty()) {
    //     latestUiDiff = diffs;
    // }

    std::cout << "[UI CAPTURE] Snapshot stored. Elements: " 
              << uiDom.size() << std::endl;
}




