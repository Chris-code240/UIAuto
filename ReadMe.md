g++ main.cpp -o main.exe -municode -lole32 -loleaut32 -luiautomationcore -lgdi32 -luser32
cl /EHsc /W4 /I include src\*.cpp main.cpp /Fe:main.exe
