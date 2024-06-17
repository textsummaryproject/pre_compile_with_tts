#include "summarizer.h"
#include <iostream>
#include <sstream>
#include <fstream>  // 추가된 부분
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstdlib>
#include "utils.h"

std::map<std::string, double> computeTFIDF(const std::vector<std::string>& doc, const std::vector<std::vector<std::string>>& corpus) {
    std::map<std::string, double> tfidf;
    std::map<std::string, int> termFrequency;
    std::map<std::string, int> documentFrequency;
    int totalTerms = doc.size();

    for (const std::string& word : doc) {
        termFrequency[word]++;
    }

    for (const std::vector<std::string>& document : corpus) {
        std::set<std::string> uniqueWords(document.begin(), document.end());
        for (const std::string& word : uniqueWords) {
            documentFrequency[word]++;
        }
    }

    for (const auto& pair : termFrequency) {
        const std::string& word = pair.first;
        int tf = pair.second;
        int df = documentFrequency[word];
        double idf = log((double)corpus.size() / (df + 1));
        tfidf[word] = (tf / (double)totalTerms) * idf;
    }

    return tfidf;
}

void textToSpeech(const std::string& text) {
    std::string command = "python text_to_speech.py \"" + text + "\"";
    std::system(command.c_str());
    std::system("start output.mp3"); // Windows에서 음성 파일 재생
}

void summarizeText(const std::string& text) {
    std::istringstream iss(text);
    std::string line;
    std::vector<std::string> sentences;
    while (std::getline(iss, line, '.')) {
        if (!line.empty()) {
            sentences.push_back(line + '.');
        }
    }

    std::vector<std::vector<std::string>> corpus;
    for (const std::string& sentence : sentences) {
        corpus.push_back(split(toLowerCase(sentence)));
    }

    std::vector<std::pair<std::string, double>> sentenceScores;
    for (const std::vector<std::string>& doc : corpus) {
        std::map<std::string, double> tfidf = computeTFIDF(doc, corpus);
        double score = 0.0;
        for (const auto& pair : tfidf) {
            score += pair.second;
        }
        sentenceScores.push_back(std::make_pair(sentences[&doc - &corpus[0]], score));
    }

    std::sort(sentenceScores.begin(), sentenceScores.end(), [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
        return b.second > a.second;
        });

    std::cout << "요약 결과:\n";
    std::vector<std::string> summary;
    for (size_t i = 0; i < std::min<size_t>(sentenceScores.size(), size_t(3)); ++i) {
        std::cout << sentenceScores[i].first << "\n";
        summary.push_back(sentenceScores[i].first);
    }

    // 요약된 내용을 하나의 문자열로 결합
    std::string fullSummary = "";
    for (const auto& sentence : summary) {
        fullSummary += sentence + " ";
    }

    textToSpeech(fullSummary);
}

void summarizeFile(const std::string& filePath) {
    std::ifstream file(filePath);  // ifstream을 사용하기 위해 <fstream>을 포함해야 합니다.
    if (!file.is_open()) {
        std::cerr << "파일을 열 수 없습니다." << std::endl;
        return;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    std::string utf8Str = ss.str(); // UTF-8 인코딩 문자열

    char* ansiStr = UTF8ToANSI(utf8Str.c_str());

    std::cout << ansiStr << std::endl; // 변환된 문자열 출력

    summarizeText(ansiStr);

    delete[] ansiStr; // 메모리 해제
}
