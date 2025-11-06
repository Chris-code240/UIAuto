#include "llama.h"
#include "ggml.h"
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

int main() {
    ggml_backend_load_all();
    ggml_backend_cpu_init();
    llama_backend_init();
    llama_model_params model_params = llama_model_default_params();
    llama_model* model = llama_model_load_from_file("model/Qwen2.5-0.5B-Q5_K_S.gguf", model_params);
    if (!model) {
        std::cerr << "❌ Failed to load model file!\n";
        llama_backend_free();
        return 1;
    }
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 512;
    llama_context* ctx = llama_init_from_model(model, ctx_params);

    if (!ctx) {
        std::cerr << "❌ Failed to init context!\n";
        llama_model_free(model);
        llama_backend_free();
        return 1;
    }

    const char* prompt = "Who are you?";

    const llama_vocab* vocab = llama_model_get_vocab(model);

    std::vector<llama_token> tokens;
    tokens.resize(strlen(prompt) * 4);

    int n_tokens = llama_tokenize(
        vocab,
        prompt,
        strlen(prompt),
        tokens.data(),
        tokens.size(),
        true,
        true
    );

    if (n_tokens < 0) {
        std::cerr << "Tokenization failed\n";
        return 1;
    }

    tokens.resize(n_tokens);


    auto sparams = llama_sampler_chain_default_params();
    llama_sampler* smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());

    std::cout << "Model reply: ";

    for (int i = 0; i < 100; ++i) {
        llama_token new_token_id = llama_sampler_sample(smpl, ctx, -1);
        if (llama_vocab_is_eog(vocab, new_token_id)) break;

        char buf[128];
        int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
        if (n < 0) break;
        std::cout.write(buf, n);

        llama_batch next_batch = llama_batch_get_one(&new_token_id, 1);
        if (llama_decode(ctx, next_batch) != 0) break;
    }

    std::cout << std::endl;
    llama_sampler_free(smpl);
    llama_free(ctx);
    llama_model_free(model);
    llama_backend_free();
    return EXIT_SUCCESS;
}
