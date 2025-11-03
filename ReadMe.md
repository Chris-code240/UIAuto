g++ main.cpp -o main.exe -municode -lole32 -loleaut32 -luiautomationcore -lgdi32 -luser32
cl /EHsc /W4 /I include src\*.cpp main.cpp /Fe:main.exe

| Layer              | Implementation                   | Runs locally? | Complexity |
| ------------------ | -------------------------------- | ------------- | ---------- |
| Input parsing      | Regex / fastText                 | ✅             | Very low   |
| Intent recognition | Rasa or small LLaMA 1B quantized | ✅             | Medium     |
| Entity extraction  | spaCy small model                | ✅             | Low        |
| Action mapping     | Rule-based C++ planner           | ✅             | Very low   |
| Execution          | Your `WindowWalker` / UIA        | ✅             | Done       |

