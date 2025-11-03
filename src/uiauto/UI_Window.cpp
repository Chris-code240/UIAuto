#include "../../include/uiauto/UI_Window.h"

UI_Automation::UIWindow::UIWindow(LPWSTR windowName){

    HRESULT hr;
    IUIAutomationCondition * pCond = nullptr;
    hr = this->IUIA->CreatePropertyCondition(UIA_NamePropertyId, _variant_t(windowName), &pCond);
     if (SUCCEEDED(hr)){
        
            hr = pRoot->FindFirst(TreeScope_Children, pCond, &this->window);
            if (FAILED(hr)){
                throw std::runtime_error("Window not found");
            }
            pCond->Release();
            pRoot->Release();
        
     }
}

void  UI_Automation::UIWindow::updateWindowSnapshot(){

    HRESULT hr;
    IUIAutomationCondition * pCond = nullptr;

    hr = this->IUIA->CreateTrueCondition(&pCond);
    if (SUCCEEDED(hr)){
        hr = this->window->FindAll(TreeScope_Children, pCond, &this->windowSnapshot);

        if (FAILED(hr)){
            throw std::runtime_error("Cannot grab window elements");
            
        }
        pCond->Release();
    }
}

IUIAutomationElementArray * UI_Automation::UIWindow::getWindowSnapshot(){
    return this->windowSnapshot;
}

IUIAutomationElementArray * UI_Automation::UIWindow::getSubElements(IUIAutomationElement *uiElement){

    HRESULT hr;
    IUIAutomationCondition * pCond = nullptr;
    IUIAutomationElementArray * subElements = nullptr;

    hr = this->IUIA->CreateTrueCondition(&pCond);
    if (SUCCEEDED(hr)){
        hr = uiElement->FindAll(TreeScope_Children, pCond, &subElements);

        if (FAILED(hr)){
            throw std::runtime_error("Cannot grab window elements");
            
        }
        pCond->Release();
        return subElements;
    } 
    return NULL; 
}

void UI_Automation::UIWindow::drawWindow(){
    HRESULT hr;
    VARIANT varHandle;
    hr = window->GetCurrentPropertyValue(UIA_NativeWindowHandlePropertyId, &varHandle);
    if (SUCCEEDED(hr) && varHandle.vt == VT_I4) {
        HWND hwnd = (HWND)(LONG_PTR)varHandle.lVal;
        SetForegroundWindow(hwnd);
    }
    if (!this->windowSnapshot){
        this->updateWindowSnapshot();
    }
    int length = 0;
    hr = this->windowSnapshot->get_Length(&length);

    for (int i = 0; i< length; i++){
        IUIAutomationElement * element = nullptr;
        this->windowSnapshot->GetElement(i, &element);
        if (element){
            RECT boundingBox;
            if (SUCCEEDED(element->get_CurrentBoundingRectangle(&boundingBox))){
                UI_Automation::DrawRect(boundingBox);
            }
            element->Release();
        }
    }
    this->windowSnapshot->Release();
}

void UI_Automation::UIWindow::writeText(std::wstring text ) {    
    if(!this->window){
        return;
    }
    HRESULT hr;
    hr = this->window->SetFocus();
    if(FAILED(hr)){
        return;
    }

    Sleep(100);


    for (wchar_t c : text) {
        INPUT ip = {0};
        ip.type = INPUT_KEYBOARD;
        ip.ki.wVk = 0;               // virtual-key not used with unicode
        ip.ki.wScan = c;             // character code
        ip.ki.dwFlags = KEYEVENTF_UNICODE;
        SendInput(1, &ip, sizeof(INPUT));

        // key release
        ip.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));

        Sleep(50); // tiny delay to make it more reliable
    }
    window->Release();
    return;
}

UI_Automation::UIWindow::~UIWindow(){}