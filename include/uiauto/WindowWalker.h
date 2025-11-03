#ifndef WINDOW_WALKER_H_INCLUDED
#define WINDOW_WALKER_H_INCLUDED

#include "./UI_Window.h"

namespace UI_Automation {

    class WindowWalker: public UIWindow{

        public:
            WindowWalker(LPWSTR windowName);

            void walker(IUIAutomationElement *ui_element);

            void positionCursor(int x, int y);

            void walk(IUIAutomationElement *start_element,IUIAutomationElement * end_element);
    };
}

#endif