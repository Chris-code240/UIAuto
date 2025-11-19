#include "./external/llama.cpp/include/llama.h"
#include "./external/llama.cpp/ggml/include/ggml.h"
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

// include
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include "external\curl\curl.h"

#include "./include/ui_manager/ui_manager.h"

// Tool function type: takes JSON args, returns JSON result
using ToolFn = std::function<json(const json&)>;

class ToolRegistry {
    std::unordered_map<std::string, ToolFn> tools;
public:
    void registerTool(const std::string &name, ToolFn fn) {
        tools[name] = fn;
    }
    bool has(const std::string &name) const {
        return tools.find(name) != tools.end();
    }
    json call(const std::string &name, const json &args) const {
        auto it = tools.find(name);
        if (it == tools.end()) return json{{"error","unknown_tool"}};
        try {
            return it->second(args);
        } catch (const std::exception &e) {
            return json{{"error","exception"}, {"message", e.what()}};
        }
    }
};





std::string generate_reply( llama_model* model, llama_context* ctx, llama_sampler* smpl, const std::string& full_prompt) {
    const llama_vocab* vocab = llama_model_get_vocab(model);

    // Tokenize the full conversation buffer
    std::vector<llama_token> tokens(full_prompt.size() * 4);

    int n_tokens = llama_tokenize( vocab, full_prompt.c_str(), full_prompt.size(), tokens.data(), tokens.size(),true, true );

    if (n_tokens < 0) {
        std::cerr << "Tokenization failed\n";
        return "";
    }

    tokens.resize(n_tokens);

    // Feed tokens into model
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    if (llama_decode(ctx, batch) != 0) {
        std::cerr << "Decode error\n";
        return "";
    }

    // Generation loop
    std::string reply;
    for (int i = 0; i < 300; i++) {  
        llama_token new_id = llama_sampler_sample(smpl, ctx, -1);

        if (llama_vocab_is_eog(vocab, new_id)) break;

        char buf[256];
        int n = llama_token_to_piece(vocab, new_id, buf, sizeof(buf), 0, true);
        if (n > 0) reply.append(buf, n);

        llama_batch next = llama_batch_get_one(&new_id, 1);
        if (llama_decode(ctx, next) != 0) break;
    }

    return reply;
}

int main() {

    ToolRegistry registry;
    UI_Automation::UIManager ui; 

    
    registry.registerTool("getWindow", [&](const json &args)->json {
        std::string windowName = args.value("window_name", "");
        if (windowName.empty()) return json{{"error","missing_arg"}, {"arg","window_name"}};
        return ui.getWindow(windowName); 
    });

    registry.registerTool("getWindowElements",[&](const json &args)->json {
        std::string windowName = args.value("window_name", "");
        if(windowName.empty()) return json{{"error","missing_arg"}, {"arg", "window_name"}};
        return ui.getWindow(windowName);
    });

    registry.registerTool("leftClick", [&](const json &args)->json{
        int x = args.value("x", NULL);
        int y = args.value("y", NULL);
        if (!x && !y) return json{{"error","missing_args"}, {"args", "x_and_y_coordinates"}};
        return ui.leftClick(x,y);
    });
    registry.registerTool("rightClick", [&](const json &args)->json{
        int x = args.value("x", NULL);
        int y = args.value("y", NULL);
        if (!x && !y) return json{{"error","missing_args"}, {"args", "x_and_y_coordinates"}};
        return ui.rightClick(nullptr,x,y);
    });
    registry.registerTool("dragAndDrop", [&](const json &args)->json{
        int from_x = args.value("x", NULL);
        int from_y = args.value("y", NULL);
        int to_x = args.value("x", NULL);
        int to_y = args.value("y", NULL);
        if (!from_x && !from_y && !to_x && !to_y) return json{{"error","missing_args"}, {"args", "from_x, from_y, to_x, to_y"}};
        return ui.dragAndDrop(from_x, from_y, to_x, to_y);
    });
    registry.registerTool("getDesktopSnapshot", [&](const json &args)->json {
        return ui.getDesktopSnapshot(); 
    });
    registry.registerTool("closeWindow", [&](const json &args)->json {
        std::string windowName = args.value("window_name","");
        if (windowName.empty()) return json{{"error","missing_arg"}, {"arg","window_name"}};
        return ui.killWindow(std::wstring (windowName.begin(), windowName.end())); 
         
    });
    // register a simple open_app tool (example)
    // Init backends
    ggml_backend_load_all();
    ggml_backend_cpu_init();
    llama_backend_init();

    // Load model
    llama_model_params model_params = llama_model_default_params();
    llama_model* model = llama_model_load_from_file(
        "model/Qwen2.5-0.5B-Q5_K_S.gguf",
        model_params
    );

    if (!model) {
        std::cerr << "Failed to load model\n";
        return 1;
    }

    // Init context
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;

    llama_context* ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        std::cerr << "Failed to init context\n";
        llama_model_free(model);
        return 1;
    }

    // Init sampler
    auto sparams = llama_sampler_chain_default_params();
    llama_sampler* smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());


    // Conversation buffer using Qwen Chat Template
    std::string conversation =
        "<|im_start|>system\n"
        "You are a helpful assistant on a windows machine (windows 11). You have functions to at your disposal to perform some tasks.\n"
        "<|im_end|>\n";

    std::cout << "Chat started. Type 'exit' to quit.\n";
    std::string user_input;
    std::cout << "\nYou: ";
    std::getline(std::cin, user_input);

    if (user_input == "exit") return EXIT_SUCCESS;

    // Add user message
    conversation += "<|im_start|>user\n" + user_input + "\n<|im_end|>\n";
    conversation += "<|im_start|>assistant\n";

    // Generate assistant reply
    std::string model_reply = generate_reply(model, ctx, smpl, conversation);
    conversation += model_reply + "<|im_end|>\n";
    while (true) {
        json parsed;
        try {
            parsed = json::parse(model_reply);
        } catch (...) {
            // model didn't return JSON -> treat as normal answer
            std::cout << "Model: " << model_reply << std::endl;
        }

        if (parsed.contains("tool")) {
            std::string tool = parsed["tool"].get<std::string>();
            json args = parsed.value("args", json::object());
            json result = registry.call(tool, args);

            // 3. Send tool_result back to model as assistant/tool run message
            std::string tool_result_msg = "TOOL_RESULT: " + result.dump();
            conversation += tool_result_msg;
            goto refeed_model;
            
        }
        // std::string user_input;
        std::cout << "\nYou: ";
        std::getline(std::cin, user_input);
        if (user_input == "exit") break;

        conversation += "<|im_start|>user\n" + user_input + "\n<|im_end|>\n";
        conversation += "<|im_start|>assistant\n";

        // Generate assistant reply
        model_reply = generate_reply(model, ctx, smpl, conversation);

        // Close assistant block
        conversation += model_reply + "<|im_end|>\n";

        

        refeed_model:
        // Add user message
        conversation += "<|im_start|>user\n" + user_input + "\n<|im_end|>\n";
        conversation += "<|im_start|>assistant\n";

        // Generate assistant reply
        std::string model_reply = generate_reply(model, ctx, smpl, conversation);
        conversation += model_reply + "<|im_end|>\n";
    }


    llama_sampler_free(smpl);
    llama_free(ctx);
    llama_model_free(model);
    llama_backend_free();
    return EXIT_SUCCESS;
}


