#include "../../include/uiauto/UIAuto.h"

UI_Automation::UIAuto::UIAuto(){
    this->IUIA = this->createUIAObject();
    if(!IUIA){
        throw std::runtime_error("Failed to create IUIAutomation Object");
    }

    HRESULT hr = this->IUIA->GetRootElement(&this->pRoot);
    if (FAILED(hr)){
        throw std::runtime_error("Root Element could not be initialized");
    }
};

UI_Automation::UIAuto::~UIAuto(){
    std::wcout<<"Destructing.."<<std::endl;
    if(this->IUIA != nullptr)        this->IUIA->Release();
    if(this->pRoot) this->pRoot->Release();
    CoUninitialize();
}

void UI_Automation::UIAuto::simulateLeftClick(int x, int y){
                SetCursorPos(x, y);

                INPUT input = {0};
                input.type = INPUT_MOUSE;

                input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &input, sizeof(INPUT));

                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &input, sizeof(INPUT));
}

bool UI_Automation::IsCOMInitialized(){
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (hr == S_FALSE) {
            // already initialized, undo the extra count
            CoUninitialize();
            return true;
        } else if (SUCCEEDED(hr)) {
            // we just initialized COM, undo it
            CoUninitialize();
            return false;
        }
        return false; // failed

    }

void UI_Automation::DrawRect(const RECT& rect, COLORREF color) {
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