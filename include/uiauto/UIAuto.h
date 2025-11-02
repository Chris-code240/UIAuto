#ifndef UIAUTO_H_INCLUDED
#define UIAUTO_H_INCLUDED
#include <windows.h>
#include <UIAutomation.h>
#include <iostream>
#include <comutil.h>


#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Uiautomationcore.lib")
#pragma comment(lib, "Gdi32.lib")


namespace UI_Automation{
    class UIAuto{
        private:
            IUIAutomation * createUIAObject(){
                CoInitialize(NULL);
                IUIAutomation *iuiaptr = nullptr;
                HRESULT hr =CoCreateInstance(CLSID_CUIAutomation8, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void **)&iuiaptr);
                if (SUCCEEDED(hr)){
                    return iuiaptr;
                }
                return nullptr;
            }
        protected:
            IUIAutomation * IUIA = nullptr;
            IUIAutomationElement * pRoot = nullptr;

        
        public:
            UIAuto();
            ~UIAuto();
            void simulateLeftClick(int x, int y);
    };
    bool IsCOMInitialized();

    void DrawRect(const RECT& rect, COLORREF color = RGB(255,0,0));

}




#endif