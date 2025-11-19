// #include "./external/llama.cpp/include/llama.h"
// #include "./external/llama.cpp/ggml/include/ggml.h"
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

// include
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

#include "./include/ui_manager/ui_manager.h"

#include "include\helpers\gemini-tools.h"

int test(int x){
    return x *2;
}

int main() {
    UI_Automation::UIManager manager;

    MSG msg = {0};

    std::cout << "Listening for UI Automation events...\n";
    int x = 0;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (x == 20) break;
        x += 1;
    }

    std::string key =  "some_api_key";
    // GeminiTools gt(key);

    // // register a tool
    // gt.tools().registerTool("get_desktop", [&](const json& args) -> json{
    //     return manager.getDesktopSnapshot().dump(4);
    // }, "Returns every opened application including Taskbar, in the form [{boundingBox:{bootom:INT, left: INT, right:INT, top:INT}, name:WINDOW_NAME},...]");

    // gt.tools().registerTool("set_window_as_foreground",[&](const json& args)-> json {
    //     std::string windowName = args.value("window_name","");
    //     return manager.selectWindow(windowName).dump(4);
    // }, "Given window_name, it sets the target window forward. Returns {success: true} if window exist and opeartion was successful else {success: false}");

    // gt.tools().registerTool("get_window", [&](const json& args)-> json {
    //     std::string windowName = args.value("window_name", "");
    //     return manager.getWindowElements(windowName, nullptr).dump(4);
    // }, "Given window_name, it returns all UI elements of the target window, in a json format: { elementName: NAME OR VALUE_BEING_HELD_BY_THE_ELEMENT, boundingBox:{bootom:INT, left: INT, right:INT, top:INT}, elementType:FOR_EXAMPLE_TEXTELEMENT, clickablePoint:{x:x, y:y}}");
    // // Send prompt
    // json result = gt.run("gemini-2.5-flash", 
    //     "Check if i have vscode opened."
    // );

    // std::cout << result.dump(4) << std::endl;
    // std::future<json> res = std::async(std::launch::async, &UI_Automation::UIManager::getWindow, &manager,"Home - File Explorer");

    // std::cout<<res.get().dump(4);
    // std::future<int> r = std::async(std::launch::async, test, 4);
    // try
    // {
    //     std::cout<<manager.getWindowElements("Lenovo Vantage").dump(4);
    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
    
    // // std::cout<<manager.getDesktopSnapshot().dump(4);
    std::cout<<manager.openedWindows.dump()<<std::endl;
    manager.~UIManager();
    return 0;
}