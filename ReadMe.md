cmake -B build -DLLAMA_CURL=OFF -DGGML_BACKEND_DL=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DGGML_CPU_ALL_VARIANTS=ON
cmake --build build --config Release

place llama.dll, ggml.ddl, ggml-base.dll, ggml-cpu-x64.dll in the same dir as amin.exe


cl /EHsc /std:c++latest /W4 ^
  /I external\llama.cpp\include ^
  /I external\llama.cpp\ggml\include ^
  /I external\json ^
  /I include\ui_manager ^
  /I include\helpers\manager_utils.h ^
  /I external\curl ^
  /I C:\opencv\build\include ^
  src\ui_manager\UI_Manager.cpp ^
 src\helpers\manager_utils.cpp ^
  main.cpp ^
  external\llama.cpp\build\src\llama.lib ^
  external\llama.cpp\build\common\common.lib ^
  external\llama.cpp\build\ggml\src\ggml.lib ^
  external\llama.cpp\build\ggml\src\ggml-base.lib ^
  external\llama.cpp\build\ggml\src\ggml-cpu-x64.lib ^
  libcurl.lib ^
  /link /OUT:main.exe ^
  /LIBPATH:\lib opencv_world4120.lib



g++ main.cpp -o main.exe -municode -lole32 -loleaut32 -luiautomationcore -lgdi32 -luser32
cl /EHsc /W4 /I include\ui_manager /I include\helpers\manager_utils.h src\ui_manager\*.cpp main.cpp /Fe:main.exe /std:c++latest

cl /EHsc /W4 /I include\ui_manager /I include\helpers\manager_utils.h /I external\json\json.hpp src\ui_manager\*.cpp main.cpp /Fe:main.exe /std:c++latest

# New Plan

1. Take screenshot
2. Send to LLM with text input
3. LLM returns coordinates of where to click (Either LeftClick or RightClick)








