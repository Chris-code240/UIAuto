#ifndef UI_MANAGER_H_INCLUDED
#define UI_MANAGER_H_INCLUDED

#include <windows.h>
#include <UIAutomation.h>
#include <iostream>
#include <comutil.h>
#include <cmath>
#include <thread>
#include <chrono>
#include <iostream>
#include <comdef.h>
#include <unordered_map>
#include "..\..\external\json\json.hpp"
#include <codecvt>
#include <locale>
#include <string>
#include <format>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <future>
#include <thread>


#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Uiautomationcore.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shell32.lib")



using json = nlohmann::json;

namespace UI_Automation {

    struct Element {
        std::wstring name;
        std::unordered_map<std::string, float> coordinates;
    };

    class UIManager {
        struct ElementInfo {
            std::string id;
            std::string name;
            CONTROLTYPEID type;
            RECT bounds;
            std::string parent;
            std::vector<std::string> children;
        };

        std::unordered_map<std::string, ElementInfo> uiDom;    // current memory
        std::unordered_map<std::string, ElementInfo> uiDomPrev; // previous snapshot

        private:
            // IUIAutomationElementArray * openedWindoes = nullptr;
            IUIAutomation * createUIAObject(){
                CoInitialize(NULL);
                IUIAutomation *iuiaptr = nullptr;
                HRESULT hr =CoCreateInstance(CLSID_CUIAutomation8, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void **)&iuiaptr);
                if (SUCCEEDED(hr)){
                    return iuiaptr;

                }else{
                    throw std::runtime_error("Could not create UIA object");
                    abort();
                }
            }
            IUIAutomation * IUIA = nullptr;
            IUIAutomationElement * pRoot = nullptr;

            json allApps = json::array();

        public:
        json openedWindows = {};
        UIManager();
        ~UIManager();

        IUIAutomation * getIUIA(){
            return this->IUIA;
        }
        IUIAutomationElement *getDesktop();
        IUIAutomationElement *getRoot();

        bool createWindow(LPWSTR windowName);

        json  getWindow(std::string windowName);
        json GetInstalledAppsFromRegistry(HKEY rootKey);

        bool killWindow(std::wstring windowName);
        json getWindowElements(std::string windowName = "", IUIAutomationElement * window = nullptr);
        bool IsSecureDesktopActive();

        // json walkWindow(IUIAutomationElement * window, IUIAutomationElement * start_element, IUIAutomationElement * end_element);
        json walkWindow();

        json leftClick(int x, int y);
        json rightClick(IUIAutomationElement * targetElement = nullptr, int x = NULL, int y = NULL);
        json hover(int x, int y);
        json getDesktopSnapshot();
        bool minimizeWindow(std::string windowName);
        bool maximizeWindow(std::string windowName);
        void DrawRect(const RECT& rect, COLORREF color = RGB(255,0,0));
        bool open_app(const std::string& appName);
        bool dragAndDrop(int from_x, int from_y, int to_x, int to_y);
        json selectWindow(std::string windowName);
        json getAllApps(){
            return this->allApps;
        }

        std::string makeStableId(IUIAutomationElement* e) {
            SAFEARRAY* sa = nullptr;
            e->GetRuntimeId(&sa);
            if (!sa) return "INVALID_ID";

            LONG l=0,u=0;
            SafeArrayGetLBound(sa,1,&l);
            SafeArrayGetUBound(sa,1,&u);

            std::string result = "el";
            for (LONG i = l; i <= u; i++) {
                int v = 0;
                SafeArrayGetElement(sa, &i, &v);
                result += "_" + std::to_string(v);
            }

            SafeArrayDestroy(sa);
            return result;
        }

        json computeDiffs();
        void captureWindow(IUIAutomationElement* win);

    };


    class WindowEventHandler : public IUIAutomationEventHandler {
    LONG _refCount = 1;
    UIManager* owner; // back-reference to manager
    public:
        
        WindowEventHandler(UIManager* mgr) : owner(mgr) {}
        // IUnknown
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&_refCount); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG val = InterlockedDecrement(&_refCount);
            if (val == 0) delete this;
            return val;
        }
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface) override {
            if (riid == IID_IUnknown || riid == __uuidof(IUIAutomationEventHandler)) {
                *ppInterface = static_cast<IUIAutomationEventHandler*>(this);
            } else {
                *ppInterface = nullptr;
                return E_NOINTERFACE;
            }
            AddRef();
            return S_OK;
        }

        // IUIAutomationEventHandler
    HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement* sender, EVENTID eventId) override {
        if (!sender) return S_OK;

        VARIANT varName;
        VariantInit(&varName);

        // Helper to get a unique runtime ID for each window
        auto getRuntimeId = [&](IUIAutomationElement* elem) -> std::string {
            SAFEARRAY* runtimeIdArray = nullptr;
            elem->GetRuntimeId(&runtimeIdArray);
            if (!runtimeIdArray) return "";

            LONG lower = 0, upper = 0;
            SafeArrayGetLBound(runtimeIdArray, 1, &lower);
            SafeArrayGetUBound(runtimeIdArray, 1, &upper);

            std::wstring idStr;
            for (LONG i = lower; i <= upper; i++) {
                int val = 0;
                SafeArrayGetElement(runtimeIdArray, &i, &val);
                idStr += std::to_wstring(val) + L"-";
            }

            SafeArrayDestroy(runtimeIdArray);
            return std::string(idStr.begin(), idStr.end());
        };

        std::string runtimeId = getRuntimeId(sender);

        if (eventId == UIA_Window_WindowOpenedEventId) {
            sender->GetCurrentPropertyValue(UIA_NamePropertyId, &varName);

            json windowJson;
            std::string windowTitle = (varName.vt == VT_BSTR && varName.bstrVal) 
                                        ? std::string(_bstr_t(varName.bstrVal)) 
                                        : "__no_name__";
            windowJson["windowTitle"] = windowTitle;

            // Collect all child elements in this window
            IUIAutomationCondition* trueCond = nullptr;
            owner->getIUIA()->CreateTrueCondition(&trueCond);

            IUIAutomationElementArray* children = nullptr;
            sender->FindAll(TreeScope_Subtree, trueCond, &children);
            if (trueCond) trueCond->Release();

            json elements = json::array();
            if (children) {
                int length = 0;
                children->get_Length(&length);

                for (int i = 0; i < length; i++) {
                    IUIAutomationElement* elem = nullptr;
                    children->GetElement(i, &elem);
                    if (!elem) continue;

                    BSTR elemName = nullptr;
                    elem->get_CurrentName(&elemName);

                    CONTROLTYPEID controlType;
                    elem->get_CurrentControlType(&controlType);

                    RECT rect;
                    elem->get_CurrentBoundingRectangle(&rect);

                    json temp = {
                        {"elementName", elemName ? std::string(_bstr_t(elemName)) : "(no name)"},
                        {"elementType", controlType},
                        {"boundingBox", {
                            {"left", rect.left}, {"top", rect.top},
                            {"right", rect.right}, {"bottom", rect.bottom}
                        }}
                    };

                    if (elemName) SysFreeString(elemName);
                    elements.push_back(temp);
                    elem->Release();
                }
                children->Release();
            }

            windowJson["elements"] = elements;

            // Store using runtimeId instead of windowTitle
            this->owner->openedWindows[runtimeId] = windowJson;
        }

        else if (eventId == UIA_Window_WindowClosedEventId) {
            if (this->owner->openedWindows.contains(runtimeId)) {
                std::string closedTitle = this->owner->openedWindows[runtimeId]["windowTitle"];
                this->owner->openedWindows.erase(runtimeId);
            } else {
                std::cout << "[!] Closed window not found (ID: " << runtimeId << ")" << std::endl;
            }
        }
        else if (eventId == UIA_MenuOpenedEventId){

        }
        else if(eventId == UIA_Invoke_InvokedEventId){

        }
        else if ( eventId == UIA_ExpandCollapseExpandCollapseStatePropertyId ){
            
        }
        else if (eventId == UIA_AutomationFocusChangedEventId){

        }

        VariantClear(&varName);
        return S_OK;
    }

};


//-------------------------------------------------------------
//  StructureChangedEventHandler
//-------------------------------------------------------------
class StructureChangedEventHandler : public IUIAutomationStructureChangedEventHandler {
    LONG _refCount = 1;
    UIManager* owner;

public:
    StructureChangedEventHandler(UIManager* mgr) : owner(mgr) {}

    //---------------------------------------------------------
    // IUnknown
    //---------------------------------------------------------
    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&_refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        LONG val = InterlockedDecrement(&_refCount);
        if (val == 0) delete this;
        return val;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface) override {
        if (riid == IID_IUnknown || 
            riid == __uuidof(IUIAutomationStructureChangedEventHandler)) {
            *ppInterface = static_cast<IUIAutomationStructureChangedEventHandler*>(this);
        } 
        else {
            *ppInterface = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    //---------------------------------------------------------
    // IUIAutomationStructureChangedEventHandler
    //---------------------------------------------------------
    HRESULT STDMETHODCALLTYPE HandleStructureChangedEvent( IUIAutomationElement* sender,  StructureChangeType changeType,    SAFEARRAY* runtimeId ) override 
    {
        if (!sender) return S_OK;

        // Convert runtime ID to string
        std::string rid = runtimeIdToString(runtimeId);

        // Fetch basic element info
        BSTR name = nullptr;
        sender->get_CurrentName(&name);

        CONTROLTYPEID ctype = 0;
        sender->get_CurrentControlType(&ctype);

        // Log basic info
        std::cout << "\n[STRUCTURE CHANGE] "
                  << "RTID = " << rid
                  << " | name = " << (name ? (const char *)&name : "(no name)")
                  << " | type = " << ctype
                  << " | changeType = " << changeType
                  << std::endl;

        // Update captured UI for the window this element belongs to
        refreshWindowForElement(sender);

        if (name) SysFreeString(name);
        return S_OK;
    }

private:

    //---------------------------------------------------------
    // Utility: convert SAFEARRAY runtime ID → string
    //---------------------------------------------------------
    std::string runtimeIdToString(SAFEARRAY* sa) {
        if (!sa) return "";
        LONG l, u;
        SafeArrayGetLBound(sa, 1, &l);
        SafeArrayGetUBound(sa, 1, &u);

        std::wstring s;
        for (LONG i = l; i <= u; i++) {
            int val = 0;
            SafeArrayGetElement(sa, &i, &val);
            s += std::to_wstring(val) + L"-";
        }
        return std::string(s.begin(), s.end());
    }

    //---------------------------------------------------------
    // Refresh the entire window that contains this element
    //---------------------------------------------------------
void refreshWindowForElement(IUIAutomationElement* elem) {
    if (!elem) return;

    IUIAutomationTreeWalker* walker = nullptr;
    HRESULT hr = owner->getIUIA()->get_ControlViewWalker(&walker);
    if (FAILED(hr) || !walker) return;

    IUIAutomationElement* current = elem;
    current->AddRef();   // because we are going to release as we climb

    while (true) {

        // Check if this is a Window
        CONTROLTYPEID ct = 0;
        current->get_CurrentControlType(&ct);

        if (ct == UIA_WindowControlTypeId) {
            // We reached the top-level window
            captureWindow(current);
            current->Release();
            walker->Release();
            return;
        }

        // Move up
        IUIAutomationElement* parent = nullptr;
        hr = walker->GetParentElement(current, &parent);

        current->Release();   // release previous

        if (FAILED(hr) || !parent) {
            // No parent -> stop
            walker->Release();
            return;
        }

        current = parent;
    }
}

    //---------------------------------------------------------
    // Captures all elements under a window (same way you do it)
    //---------------------------------------------------------
    void captureWindow(IUIAutomationElement* win) {
        // This duplicates your window-element collection from WindowEventHandler
        // You may refactor this to call your existing code.

        BSTR name = nullptr;
        win->get_CurrentName(&name);

        json winJson;
        winJson["windowTitle"] = name ? std::string(_bstr_t(name)) : "__no_name__";

        // Query all elements inside window
        IUIAutomationCondition* trueCond = nullptr;
        owner->getIUIA()->CreateTrueCondition(&trueCond);

        IUIAutomationElementArray* elems = nullptr;
        win->FindAll(TreeScope_Subtree, trueCond, &elems);
        if (trueCond) trueCond->Release();

        json arr = json::array();

        if (elems) {
            int length = 0;
            elems->get_Length(&length);

            for (int i = 0; i < length; i++) {
                IUIAutomationElement* e = nullptr;
                elems->GetElement(i, &e);
                if (!e) continue;

                BSTR en = nullptr;
                e->get_CurrentName(&en);

                CONTROLTYPEID ct = 0;
                e->get_CurrentControlType(&ct);

                RECT r;
                e->get_CurrentBoundingRectangle(&r);

                arr.push_back({
                    {"elementName", en ? std::string(_bstr_t(en)) : "(no name)"},
                    {"elementType", ct},
                    {"boundingBox", {
                        {"left", r.left}, {"top", r.top},
                        {"right", r.right}, {"bottom", r.bottom}
                    }}
                });

                if (en) SysFreeString(en);
                e->Release();
            }
            elems->Release();
        }

        // Store the window under its runtime ID
        SAFEARRAY* sa = nullptr;
        win->GetRuntimeId(&sa);
        std::string rid = runtimeIdToString(sa);
        SafeArrayDestroy(sa);

        owner->openedWindows[rid] = winJson;
        owner->openedWindows[rid]["elements"] = arr;

        std::cout << "\n[UPDATED WINDOW] " 
                  << winJson["windowTitle"] 
                  << " | runtimeID =" << rid
                  << std::endl;

        if (name) SysFreeString(name);
    }


};




}

#endif