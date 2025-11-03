#include "./include/uiauto/UI_Window.h"
#include "./include/helpers/text2speech.h"


int main() {
    UI_Automation::UIWindow pElement(L"Untitled - Notepad");
    pElement.writeText(L"Hello\n");

    return EXIT_SUCCESS;
}
