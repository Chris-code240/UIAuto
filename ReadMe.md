cmake -B build -DLLAMA_CURL=OFF -DGGML_BACKEND_DL=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DGGML_CPU_ALL_VARIANTS=ON
cmake --build build --config Release

place llama.dll, ggml.ddl, ggml-base.dll, ggml-cpu-x64.dll in the same dir as amin.exe


cl /EHsc /std:c++latest /W4 ^
  /I external\llama.cpp\include ^
  /I external\llama.cpp\ggml\include ^
  /I external\json\json.hpp ^
  /I include\ui_manager ^
  /I external\curl\ ^
  src\ui_manager\UI_Manager.cpp ^
  .\main.cpp ^
  external\llama.cpp\build\src\llama.lib ^
  external\llama.cpp\build\common\common.lib ^
  external\llama.cpp\build\ggml\src\ggml.lib ^
  external\llama.cpp\build\ggml\src\ggml-base.lib ^
  external\llama.cpp\build\ggml\src\ggml-cpu-x64.lib ^
  .\libcurl.lib ^
  /link /OUT:main.exe


g++ main.cpp -o main.exe -municode -lole32 -loleaut32 -luiautomationcore -lgdi32 -luser32
cl /EHsc /W4 /I include\ui_manager /I include\helpers\manager_utils.h src\ui_manager\*.cpp main.cpp /Fe:main.exe /std:c++latest

cl /EHsc /W4 /I include\ui_manager /I include\helpers\manager_utils.h /I external\json\json.hpp src\ui_manager\*.cpp main.cpp /Fe:main.exe /std:c++latest

| Layer              | Implementation                   | Runs locally? | Complexity |
| ------------------ | -------------------------------- | ------------- | ---------- |
| Input parsing      | Regex / fastText                 | ✅             | Very low   |
| Intent recognition | Rasa or small LLaMA 1B quantized | ✅             | Medium     |
| Entity extraction  | spaCy small model                | ✅             | Low        |
| Action mapping     | Rule-based C++ planner           | ✅             | Very low   |
| Execution          | Your `WindowWalker` / UIA        | ✅             | Done       |





