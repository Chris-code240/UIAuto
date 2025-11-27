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
// #include ".\include\helpers\manager_utils.h"
#include "include\helpers\gemini-tools.h"
#include <windows.h>
#include "external\opencv2\opencv.hpp"

using namespace cv;

//std::wstring ws =   std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes();
//

int test(int &x){
   return x++;
}

int mai0n() {
    UI_Automation::UIManager manager;
    std::cout<<manager.getDesktopSnapshot().dump(4);
    std::string key = "Some_API_KEY";
    GeminiTools gt(key, "", manager.memoryToJson());
    // manager.hover(500,845);Sleep(150);
    // // std::cout<<manager.getAllApps();
    // std::string search_ter = "notepad", inputText = "How lazy can I be?";
    // POINT pt; pt.x = 571; pt.y =840;
    // manager.hover(pt.x, pt.y);
    // std::cout<<manager.leftClick(pt.x, pt.y).dump();
    // manager.inputText(pt, search_ter);
    // Sleep(500);
    // manager.hover(500,500);
    // manager.leftClick(500, 500);
    // Sleep(120);
    // POINT p2; p2.x= 500; p2.y= 500;
    // manager.inputText(p2, inputText);
    // Sleep(200);

    // std::cout<<manager.computeDiffs().dump(4);


    // IUIAutomationCondition * t = nullptr;
    // IUIAutomationElementArray * eles = nullptr;
    // manager.getIUIA()->CreateTrueCondition(&t);
    // manager.getRoot()->FindAll(TreeScope_Children, t, &eles);

    // int l = 0;
    // eles->get_Length(&l);
    // std::cout<<l;
   
/*
{
        "boundingBox": {
            "bottom": 864,
            "left": 0,
            "right": 1536,
            "top": 816
        },
        "name": "Taskbar"
}
        */
    // json d = manager.getWindowElements("Taskbar", nullptr).dump(4);

    // std::cout<<d;

    // MSG msg = {0};

    // std::cout << "Listening for UI Automation events...\n";
    // int x = 0;

    // while (GetMessage(&msg, NULL, 0, 0)) {
    //     TranslateMessage(&msg);
    //     DispatchMessage(&msg);
    //     if (x == 100){ std::cout<<"=======Computing============\n";
    //          break;}
    //     x += 1;
    // }
    // for (auto [id, data] : manager.memory){
    //     std::cout<<"id => "<<id<<" Title => "<<data.title<<"Root => "<<data.root<<std::endl;
        
    // }
    // std::string key =  "some_api_key";
    // GeminiTools gt(key);

    // register a tool
    gt.tools().registerTool("get_desktop", [&](const json& args) -> json{
        CoInitialize(NULL);

        std::future<json> res = std::async(std::launch::async, &UI_Automation::UIManager::memoryToJson, &manager);
        CoUninitialize();
        return res.get();
        // return manager.getDesktopSnapshot().dump(4);
    }, "Returns every opened application including Taskbar, with their sub-elements and bound coordinates, in the form [{runtimeId:some_string_id, tree:{boundingBox:{bootom:INT, left: INT, right:INT, top:INT}, children:[...]}, windowTitle:WINDOW_NAME},...]");

    gt.tools().registerTool("set_window_as_foreground",[&](const json& args)-> json {
        std::string windowName = args.value("window_name","");
        return manager.selectWindow(windowName).dump(4);
    }, "Given window_name, it sets the target window forward. Returns {success: true} if window exist and opeartion was successful else {success: false}");
    
    // gt.tools().registerTool("get_all_apps",[&](const json& args)-> json {
    //     return manager.getAllApps().dump(4);
    // }, "Returns all apps installed on the machine in a json format: {appName:{path:PATH_TO_APP, version:APP_VERSION},...}");

    // gt.tools().registerTool("launch_app",[&](const json& args)-> json {
    //     std::string app_name = args.value("app_name","");
    //     return manager.open_app(app_name);
    // }, "Given app_name, it laucnhes the app and returns true if successfull else false.");

    gt.tools().registerTool("left_click",[&](const json& args)-> json {
        CoInitialize(NULL);

        int x = args.value("x",NULL);
        int y = args.value("y",NULL);
        manager.leftClick(x,y);
        CoUninitialize();
        return manager.memoryToJson();
    }, "Given x and y coordiantes, it simulates left click and returns a json of entire screen.");
    gt.tools().registerTool("hover_pointer",[&](const json& args)-> json {
        CoInitialize(NULL);

            int x = args.value("x",NULL);
            int y = args.value("y",NULL);
            bool leftClick = args.value("leftClick", false);
            manager.hover(x, y);
            if(leftClick){
                manager.leftClick(x,y);
            }
            CoUninitialize();
            return manager.memoryToJson();
    }, "Given x and y coordiantes, it sends the mouse cursor the goiven coordinates. If leftClick is given as true, it performs simualtes left click thereafter");
        
    gt.tools().registerTool("input_text",[&](const json& args)-> json {

        CoInitialize(NULL);
        int x = args.value("x",NULL);
        int y = args.value("y",NULL);
        std::string text = args.value("text", "");
        POINT pt; pt.x = x, pt.y = y;
        manager.inputText(pt, text);
        CoUninitialize();
        return manager.memoryToJson();
    }, "Given x and y coordiantes, and text it simulates text input into target input field element");

    // gt.tools().registerTool("get_window", [&](const json& args)-> json {
    //     std::string windowName = args.value("window_name", "");
    //     return manager.getWindowElements(windowName, nullptr).dump(4);
    // }, "Given window_name, it returns all UI elements of the target window, in a json format: { elementName: NAME OR VALUE_BEING_HELD_BY_THE_ELEMENT, boundingBox:{bootom:INT, left: INT, right:INT, top:INT}, elementType:FOR_EXAMPLE_TEXTELEMENT, clickablePoint:{x:x, y:y}}");
    // Send prompt
    CoInitialize(NULL);
    std::cout<<manager.getDesktopSnapshot().dump(4);
    CoUninitialize();
//     while(true){
//         std::cout<<"You: ";
//         std::string userInput;
//         std::getline(std::cin, userInput);

//         if (userInput == "q"){break;}
//     json result = gt.run("gemini-2.5-flash", 
//         userInput
//     );
//     std::cout<<"\n"<<result.dump(4)<<std::endl;
// }

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
    // std::cout<<manager.computeDiffs().dump(4)<<std::endl;
    // std::wcout<<L"Old = "<<manager.uiDomPrev.size()<<L" New = "<<manager.uiDom.size();
    manager.~UIManager();
    // int y = 5;
    // std::cout<<test(y)<<" "<<y;
    return 0;

}

// -----------------------------
// MAIN EXAMPLE
// -----------------------------
int main()
{
    cv::Mat screenshot = takeScreenshot();

    if (screenshot.empty()) {
        std::cout << "Failed to capture screen\n";
        return -1;
    }

    drawGrid(screenshot);

    imwrite("desktop_grid.jpg", screenshot);

    imshow("Desktop with Grid", screenshot);
    waitKey(0);
    return 0;
}