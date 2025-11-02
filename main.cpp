#include "./include/uiauto/UI_Window.h"

int main() {

    try
    {
        UI_Automation::UIWindow  window(L"Home - File Explorer");
        for(int i = 0;i < 10; i++){
            window.drawWindow();

            Sleep(10000);
        }
        // window.~UIWindow();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    

    return 0;
}
