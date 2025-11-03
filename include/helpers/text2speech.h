#ifndef TEXT_2_SPEECH_INCLUDED
#define TEXT_2_SPEECH_INCLUDED
#include <windows.h>
#include <sapi.h>
#pragma comment(lib, "sapi.lib")
#pragma comment(lib, "ole32.lib")


namespace UI_Automation{

    int text_to_speech(LPWSTR text);
}

#endif