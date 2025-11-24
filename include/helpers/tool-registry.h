#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include "..\..\external\json\json.hpp"
#include <iostream>
#include <sstream>

using json = nlohmann::json;
using ToolFn = std::function<json(const json&)>;


struct ToolInfo {
    std::string description;
    ToolFn fn;
};

class ToolRegistry {
    json desktop = json::array();
    std::unordered_map<std::string, ToolInfo> tools;

public:
    ToolRegistry(json _desktop = json::array()){ this->desktop = _desktop;}
    void registerTool(const std::string &name,
                      ToolFn fn,
                      const std::string &description)
    {
        tools[name] = ToolInfo{ description, fn };
    }

    bool has(const std::string &name) const {
        return tools.find(name) != tools.end();
    }

    json call(const std::string &name, const json &args) const {
        auto it = tools.find(name);
        if (it == tools.end())
            return json{{"error","unknown_tool"}};

        try {
            return it->second.fn(args);
        } catch (const std::exception &e) {
            return json{
                {"error","exception"},
                {"message", e.what()}
            };
        }
    }

    // Describe tools to place in the Gemini system prompt
    std::string toolsDescription() const {
        std::ostringstream ss;
        ss << "Available tools:\n";

        for (const auto &kv : tools) {
            ss << "- " << kv.first << ": " << kv.second.description << "\n";
        }

        ss << "\nYou are a helpful desktop assistant on a Windows 11 laptop. You have toools at your disposal to perform some tasks.\nThis is the initial desktop view in a json format:\n"<<this->desktop<<"\nWhen you want to call a tool, return ONLY this JSON:\n"
           << "{\"functionCall\": {\"name\": \"tool_name\", \"args\": {...}}}\n"
           << "Never include extra text outside that JSON.\n";

        return ss.str();
    }
};

