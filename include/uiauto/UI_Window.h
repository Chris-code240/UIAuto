#ifndef UI_WINDOW_H_INCLUDED
#define UI_WINDOW_H_INCLUDED

#include "./UIAuto.h"

namespace  UI_Automation{

    class UIWindow: public UIAuto {
        private:
            IUIAutomationElement * window = nullptr;
            IUIAutomationElementArray * windowSnapshot = nullptr;
        public:
            UIWindow(LPWSTR windowName);
            ~UIWindow();

            void drawWindow();
            IUIAutomationElementArray * getWindowSnapshot();
            void updateWindowSnapshot();
            IUIAutomationElementArray * getSubElements(IUIAutomationElement *uiElement);
    };
    
} 


#endif