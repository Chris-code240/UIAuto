#pragma once
#include <string>
#include "..\..\external\json\json.hpp"
#include "..\..\external\curl\curl.h"

using json = nlohmann::json;
class GeminiClient {
    std::string api_key;
    std::string base_url; // e.g. https://generativelanguage.googleapis.com/v1beta/
public:
    GeminiClient(const std::string &key,
                 const std::string &base = "https://generativelanguage.googleapis.com/v1beta/")
        : api_key(key), base_url(base) {}

    // send a JSON payload to models/<model>:generateContent?key=API_KEY
    // returns parsed json or error json
    json sendGenerateContent(const std::string &model, const json &payload, long timeoutSeconds = 20) {
        std::string url = base_url + "models/" + model + ":generateContent?key=" + api_key;
        std::string payloadStr = payload.dump();

        CURL *curl = curl_easy_init();
        if (!curl) return json{{"error","curl_init_failed"}};

        std::string response;
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);

        // write callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* contents, size_t size, size_t nmemb, void* userp)->size_t {
            size_t realSize = size * nmemb;
            std::string *s = (std::string*)userp;
            s->append((char*)contents, realSize);
            return realSize;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return json{{"error","curl_error"}, {"message", curl_easy_strerror(res)}};
        }

        try {
            auto parsed = json::parse(response);
            // attach http code for debugging
            parsed["__http_status"] = httpCode;
            return parsed;
        } catch (const std::exception &e) {
            return json{{"error","invalid_json_response"}, {"raw", response}, {"message", e.what()}, {"http_code", httpCode}};
        }
    }
};
