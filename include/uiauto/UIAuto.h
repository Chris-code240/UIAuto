#ifndef UIAUTO_H_INCLUDED
#define UIAUTO_H_INCLUDED

#include "../ui_manager/UI_Manager.h"

namespace UI_Automation{
    class UIAuto : public UI_Automation::UIManager{
        public:
            UIAuto();
            ~UIAuto();
            void simulateLeftClick(int x, int y);
    };
    bool IsCOMInitialized();

    void DrawRect(const RECT& rect, COLORREF color = RGB(255,0,0));

}




#endif