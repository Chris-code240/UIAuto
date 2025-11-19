#pragma once
#include "Gemini-Client.h"
#include "Tool-Registry.h"
// gemini_tools.cpp
// Single-file example: Gemini client + tool registry + tool-calling loop
// Requires: libcurl, nlohmann::json
// Compile with: cl /EHsc gemini_tools.cpp /I"<vcpkg>/installed/x64-windows/include" /link "<vcpkg>/installed/x64-windows/lib/libcurl.lib"

#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <ctime>
#include <sstream>

using json = nlohmann::json;

class GeminiTools {
    GeminiClient client;
    ToolRegistry registry;
    std::string systemHeader; // optional global instructions

public:
    GeminiTools(const std::string &apiKey, const std::string &systemHeader_ = "")
        : client(apiKey), systemHeader(systemHeader_) {}

    ToolRegistry & tools() { return registry; }

    // Create the "contents" array with system pre-header (tool descriptions + optional system header)
    json makeBasePayload(const std::string &userMessage) const {
        std::string pre = systemHeader;
        if (!pre.empty()) pre += "\n\n";
        pre += registry.toolsDescription();
        pre += "\nUser message follows:\n";

        json payload;
        payload["contents"] = json::array();
        // First content part: system + tool schema
        json contentSystem;
        contentSystem["parts"] = json::array({
            json{{"text", pre}}
        });
        payload["contents"].push_back(contentSystem);

        // Second: actual user message
        json contentUser;
        contentUser["parts"] = json::array({
            json{{"text", userMessage}}
        });
        payload["contents"].push_back(contentUser);

        return payload;
    }

    // parse functionCall from Gemini response if present.
    // returns { found:bool, name:string, args:json }
    static std::tuple<bool, std::string, json> extractFunctionCall(const json &response) {
        try {
            // The Gemini responses we've used have array: candidates -> [ { content: { parts: [ { text: "...", maybe functionCall? } ] } } ]
            // We'll inspect candidates[0].content.parts[*] for functionCall field.
            if (!response.contains("candidates")) return {false, "", {}};
            auto cand = response.at("candidates").at(0);
            if (!cand.contains("content")) return {false, "", {}};
            auto content = cand.at("content");
            std::cout<<"Processing first request..\n";
            // parts may be an array of objects; each part might contain functionCall
            if (content.contains("parts") && content.at("parts").is_array()) {
                for (auto &p : content.at("parts")) {
                    if (p.is_object() && p.contains("functionCall")) {
                        auto fc = p.at("functionCall");
                        std::string name = fc.value("name", "");
                        json args = fc.value("args", json::object());
                        // some models may return args as string; try parse if so
                        if (args.is_string()) {
                            try {
                                args = json::parse(args.get<std::string>());
                            } catch (...) { /* leave as string */ }
                        }
                        return {true, name, args};
                    }
                    // fallback: some responses may embed the functionCall inside text as raw JSON
                    if (p.is_object() && p.contains("text") && p["text"].is_string()) {
                        std::string t = p["text"];
                        std::cout<<t;

                        // crude search for {"functionCall":
                        auto pos = t.find("\"functionCall\"");
                        if (pos != std::string::npos) {

                            // try to extract JSON substring (best-effort)
                            auto open = t.find('{', pos);
                            auto close = t.rfind('}');
                            if (open != std::string::npos && close != std::string::npos) {
                            
                                // fallback: some responses may embed the functionCall inside text as raw JSON
                                if (p.is_object() && p.contains("text") && p["text"].is_string()) {
                                    std::string t = p["text"];
                                    std::cout<<t;

                                    try {
                                        // Attempt to parse the whole string t.
                                        // If it was the correct object, the unescape should happen here.
                                        json parsed = json::parse(t); 

                                        if (parsed.contains("functionCall")) {
                                            auto fc = parsed["functionCall"];
                                            std::string name = fc.value("name", "");
                                            json args = fc.value("args", json::object());
                                            // ... (handle string args if needed)
                                            return {true, name, args};
                                        }
                                    } catch (const std::exception &e) { /* Fall through if parse fails */ }

                                
                                }
                                // ...
                            }
                        }
                    }
                }
            }
        } catch (...) {}
        return {false, "", {}};
    }

    // The main entry: runs up to two passes.
    // 1) Sends user prompt (with system + tool descriptions).
    // 2) If Gemini calls a tool, execute it locally and send a follow-up with the tool result for final answer.
json run(const std::string &model, const std::string &userMessage) 
{
    json out;                      // final output container
    json currentResp;              // the latest model response
    json toolResult;               // last tool output
    std::string fname;             // function name
    json fargs;                    // function args
    bool toolExecutedOnce = false; // to include metadata

    // ----------------------------------------------------
    // 1) Send initial request
    // ----------------------------------------------------
    {
        json payload = makeBasePayload(userMessage);
        currentResp = client.sendGenerateContent(model, payload);

        if (currentResp.contains("error")) {
            out["error"] = currentResp;
            goto FINISH;
        }
        goto CHECK_FUNCALL;
    }

CHECK_FUNCALL:
    {
        // Extract function call
        auto [found, name, args] = extractFunctionCall(currentResp);

        if (!found) {
            // no tool call, we are done
            out["initial_response"] = currentResp;
            out["final_response"]   = currentResp;
            goto FINISH;
        }

        fname = name;
        fargs = args;
        toolExecutedOnce = true;

        // Tool not registered
        if (!registry.has(fname)) {
            out = json{
                {"error", "tool_not_registered"},
                {"tool", fname},
                {"original_response", currentResp}
            };
            goto FINISH;
        }

        // ----------------------------------------------------
        // 2) Execute tool
        // ----------------------------------------------------
        toolResult = registry.call(fname, fargs);

        // ----------------------------------------------------
        // 3) Build follow-up request
        // ----------------------------------------------------
        json followup;
        followup["contents"] = json::array();

        // System + tools
        {
            json contentSystem;
            std::string pre = systemHeader;
            if (!pre.empty()) pre += "\n\n";
            pre += registry.toolsDescription();
            contentSystem["parts"] = json::array({ json{{"text", pre}} });
            followup["contents"].push_back(contentSystem);
        }

        // Original user message
        {
            json contentUser;
            contentUser["parts"] = json::array({ json{{"text", userMessage}} });
            followup["contents"].push_back(contentUser);
        }

        // Assistant: functionCall description
        {
            json assistantCall;
            json fcPart;
            fcPart["functionCall"] = { {"name", fname}, {"args", fargs} };
            assistantCall["parts"] = json::array({ fcPart });
            followup["contents"].push_back(assistantCall);
        }

        // Tool result
        {
            json toolMsg;
            std::ostringstream ts;
            ts << "Tool '" << fname << "' returned: " << toolResult.dump()
               << "\n\nPlease provide a final answer to the user, incorporating the tool's result. "
               << "Do not call further tools unless absolutely necessary.";
            toolMsg["parts"] = json::array({ json{{"text", ts.str()}} });
            followup["contents"].push_back(toolMsg);
        }

        // ----------------------------------------------------
        // 4) Call model again with augmented context
        // ----------------------------------------------------
        currentResp = client.sendGenerateContent(model, followup);

        if (currentResp.contains("error")) {
            out = currentResp;
            goto FINISH;
        }

        // If the model again calls a tool → repeat the cycle
        goto CHECK_FUNCALL;
    }

FINISH:
    {
        if (toolExecutedOnce) {
            out["tool_executed"] = fname;
            out["tool_args"]     = fargs;
            out["tool_result"]   = toolResult;
        }
        return out;
    }
}

};
