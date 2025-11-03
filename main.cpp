#include "./include/helpers/text2speech.h"
#include "./external/fasttext/fasttext.h"

int main() {


fasttext::FastText model;
model.loadModel("intent_model.bin");

std::vector<std::pair<fasttext::real, std::string>> predictions;
model.predict("maximize this window", 1, predictions);

std::cout << predictions[0].second << std::endl;
}
