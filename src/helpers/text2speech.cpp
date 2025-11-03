#include "../../include/helpers/text2speech.h"

int UI_Automation::text_to_speech(LPWSTR text) {
   ::CoInitialize(NULL);
   ISpVoice * pVoice = NULL;
   if (FAILED(::CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice))) {
       return -1;
   }
   pVoice->Speak(text, SPF_IS_XML, NULL);
   pVoice->Release();
   pVoice = NULL;
   ::CoUninitialize();
   return 0;
}