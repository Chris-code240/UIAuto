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

#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Uiautomationcore.lib")
#pragma comment(lib, "Gdi32.lib")


#include <vector>
namespace UI_Automation {

    struct Element {
        LPWSTR name;
        RECT coordinates;
    };
    class UIManager {

        private:
            IUIAutomationElementArray * openedWindoes = nullptr;
            IUIAutomation * createUIAObject(){
                CoInitialize(NULL);
                IUIAutomation *iuiaptr = nullptr;
                HRESULT hr =CoCreateInstance(CLSID_CUIAutomation8, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void **)&iuiaptr);
                if (SUCCEEDED(hr)){
                    return iuiaptr;
                }
                return nullptr;
            }
            IUIAutomation * IUIA = nullptr;
            IUIAutomationElement * pRoot = nullptr;

        public:

        UIManager();
        ~UIManager();

        IUIAutomationElement *getDesktop();
        IUIAutomationElement *getRoot();

        bool createWindow(LPWSTR windowName);

        IUIAutomationElement * getWindow(LPWSTR windowName);

        bool killWindow(LPWSTR windowName);

        bool walkWindow(IUIAutomationElement * window, IUIAutomationElement * start_element, IUIAutomationElement * end_element);

        bool Click(IUIAutomationElement * window, IUIAutomationElement * targetElement, bool leftClick = true);

        std::vector<Element> getDesktopSnapshot();

    };
}

#endif