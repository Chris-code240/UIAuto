#include "../../include/ui_manager/UI_Manager.h"
#include <format>
#include <string>
#include <iostream>

using namespace UI_Automation;

UIManager::UIManager(){

    HRESULT hr;
    this->IUIA = this->createUIAObject();
    if (! this->IUIA){
        throw std::runtime_error("IUIAutomation object could not be created.");
    }

    hr = this->IUIA->GetRootElement(&this->pRoot);
    if (FAILED(hr)){
        throw std::runtime_error("UIManager could not grab Root Element");
    }
}

UIManager::~UIManager(){
    if (this->IUIA != nullptr) this->IUIA->Release();
    if (this->pRoot != nullptr)  this->pRoot->Release();
}

IUIAutomationElement * UIManager::getDesktop(){
    HRESULT hr;
    hr = this->IUIA->GetRootElement(&this->pRoot);
    if (FAILED(hr)) return nullptr;
    return this->pRoot;
}

IUIAutomationElement * UIManager::getRoot(){
    HRESULT hr;
    hr = this->IUIA->GetRootElement(&this->pRoot);
    if (FAILED(hr)) return nullptr;
    return this->pRoot;
}

std::vector<Element> UIManager::getDesktopSnapshot(){

    HRESULT hr;
    IUIAutomationCondition * pCond = nullptr;
    IUIAutomationElementArray * subElements = nullptr;
    std::vector<Element> elementList;
    hr = this->IUIA->CreateTrueCondition(&pCond);
    if (SUCCEEDED(hr)){
        hr = this->getDesktop()->FindAll(TreeScope_Children, pCond, &subElements);

        if (FAILED(hr)){
            throw std::runtime_error("Cannot grab window elements");
            
        }
        pCond->Release();
        int length;
        if (FAILED(subElements->get_Length(&length))) throw std::runtime_error("Could not get Desktop Elements Lenght");

        for (int i = 0; i<length; i++){
            IUIAutomationElement * element = nullptr;
            if (FAILED (subElements->GetElement(i,&element))){ 
                throw std::runtime_error("C");
            }
            VARIANT varHandle;
            RECT boundingBox;
            if (FAILED(element->GetCurrentPropertyValue(UIA_NamePropertyId,&varHandle))) throw std::runtime_error("Property Value");
            if (FAILED(element->get_CurrentBoundingRectangle(&boundingBox))) throw std::runtime_error("BoundingBox Value");
            assert(varHandle.bstrVal != nullptr);
            Element e = {varHandle.bstrVal, boundingBox};
            elementList.push_back(e);

            if (element) element->Release();
        }
        subElements->Release();

        return elementList;
    } 
    
    throw std::runtime_error("Something occurred when getting snapshot");
    
}

IUIAutomationElement * UIManager::getWindow(LPWSTR windowName){

    IUIAutomationCondition * pCond = nullptr;
    IUIAutomationElement *foundElement = nullptr;
    HRESULT hr = this->IUIA->CreatePropertyCondition(UIA_NamePropertyId, _variant_t(windowName), &pCond);
    if ( FAILED(hr) ) throw std::runtime_error("Error getting window");
    if (SUCCEEDED(this->getDesktop()->FindFirst(TreeScope_Children,pCond,&foundElement))) {
        pCond->Release();
        return foundElement;
    }
    throw std::runtime_error("Window not found. Try opening it."); // @TODO: open it instead of throwing error
    
}