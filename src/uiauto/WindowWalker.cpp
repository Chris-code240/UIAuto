#include "../../include/uiauto/WindowWalker.h"

UI_Automation::WindowWalker::WindowWalker(LPWSTR windowName): UIWindow(windowName){

}

void UI_Automation::WindowWalker::walker(IUIAutomationElement *ui_element){

    HRESULT hr;
    RECT boundingBox;
    hr  = ui_element->get_CurrentBoundingRectangle(&boundingBox);

    if (SUCCEEDED(hr)){
        this->positionCursor(boundingBox.left, boundingBox.top);
        UI_Automation::DrawRect(boundingBox);
    }else{
        throw std::runtime_error("Could not draw bounding box");
    }

}

void UI_Automation::WindowWalker::positionCursor(int x, int y){
    SetCursorPos(x, y);
}

void UI_Automation::WindowWalker::walk(IUIAutomationElement *start_element,IUIAutomationElement * end_element){
    if (!end_element) {
        throw std::runtime_error( "[walk] Invalid element pointers.\n");
    }

    if (!start_element)    start_element = this->window;


    // Get bounding rectangles
    RECT start_rect{}, end_rect{};
    HRESULT hr1 = start_element->get_CurrentBoundingRectangle(&start_rect);
    HRESULT hr2 = end_element->get_CurrentBoundingRectangle(&end_rect);

    if (FAILED(hr1) || FAILED(hr2)) {
        throw std::runtime_error("[walk] Failed to retrieve bounding rectangles.\n");
    }

    // Compute centers
    POINT start_pt{
        static_cast<LONG>((start_rect.left + start_rect.right) / 2),
        static_cast<LONG>((start_rect.top + start_rect.bottom) / 2)
    };

    POINT end_pt{
        static_cast<LONG>((end_rect.left + end_rect.right) / 2),
        static_cast<LONG>((end_rect.top + end_rect.bottom) / 2)
    };

    // Parameters for smooth movement
    const int steps = 100;         // number of interpolation steps
    const int delay_ms = 5;        // delay between steps (controls speed)
    
    auto easeInOut = [](double t) {
        // simple cubic easing
        return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
    };

    // Move smoothly
    for (int i = 0; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        double eased = easeInOut(t);

        int x = static_cast<int>(start_pt.x + (end_pt.x - start_pt.x) * eased);
        int y = static_cast<int>(start_pt.y + (end_pt.y - start_pt.y) * eased);

        SetCursorPos(x, y);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    // Ensure we end exactly at the final element center
    SetCursorPos(end_pt.x, end_pt.y);
}