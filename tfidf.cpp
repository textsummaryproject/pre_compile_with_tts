#include <opencv2/opencv.hpp>
#include <windows.h>
#include <iostream>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <sapi.h>

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
using namespace std;

// UTF-8 문자열을 ANSI 문자열로 변환하는 함수
char* UTF8ToANSI(const char* pszCode) {
    BSTR bstrWide;
    char* pszAnsi;
    int nLength;

    nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, NULL, NULL);
    bstrWide = SysAllocStringLen(NULL, nLength);

    MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, bstrWide, nLength);

    nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
    pszAnsi = new char[nLength];

    WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
    SysFreeString(bstrWide);

    return pszAnsi;
}

// 단어를 소문자로 변환하는 함수
std::string toLowerCase(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// 문자열을 단어로 분할하는 함수
std::vector<std::string> split(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// TF-IDF 계산 함수
std::map<std::string, double> computeTFIDF(const std::vector<std::string>& doc, const std::vector<std::vector<std::string>>& corpus) {
    std::map<std::string, double> tfidf;
    std::map<std::string, int> termFrequency;
    std::map<std::string, int> documentFrequency;
    int totalTerms = doc.size();

    // TF 계산
    for (const std::string& word : doc) {
        termFrequency[word]++;
    }

    // DF 계산
    for (const std::vector<std::string>& document : corpus) {
        std::set<std::string> uniqueWords(document.begin(), document.end());
        for (const std::string& word : uniqueWords) {
            documentFrequency[word]++;
        }
    }

    // TF-IDF 계산
    for (const auto& pair : termFrequency) {
        const std::string& word = pair.first;
        int tf = pair.second;
        int df = documentFrequency[word];
        double idf = log((double)corpus.size() / (df + 1));
        tfidf[word] = (tf / (double)totalTerms) * idf;
    }

    return tfidf;
}

void summarizeText(const std::string& text) {
    std::istringstream iss(text);
    std::string line;
    std::vector<std::string> sentences;
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            sentences.push_back(line);
        }
    }

    // 문서를 단어로 분할
    std::vector<std::vector<std::string>> corpus;
    for (const std::string& sentence : sentences) {
        corpus.push_back(split(toLowerCase(sentence)));
    }

    // 각 문장의 TF-IDF 계산
    std::vector<std::pair<std::string, double>> sentenceScores;
    for (const std::vector<std::string>& doc : corpus) {
        std::map<std::string, double> tfidf = computeTFIDF(doc, corpus);
        double score = 0.0;
        for (const auto& pair : tfidf) {
            score += pair.second;
        }
        sentenceScores.push_back(std::make_pair(sentences[&doc - &corpus[0]], score));
    }

    // TF-IDF 점수가 높은 순으로 문장 정렬
    std::sort(sentenceScores.begin(), sentenceScores.end(), [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
        return b.second > a.second;
        });

    // 요약 결과 출력 (상위 3문장)
    std::cout << "요약 결과:\n";
    std::vector<std::string> summary;
    for (size_t i = 0; i < std::min<size_t>(sentenceScores.size(), size_t(3)); ++i) {
        std::cout << sentenceScores[i].first << "\n";
        summary.push_back(sentenceScores[i].first);
    }

    // TTS로 요약된 내용 읽기
    ISpVoice* pVoice = nullptr;

    if (FAILED(::CoInitialize(nullptr))) {
        std::cerr << "COM 라이브러리를 초기화할 수 없습니다." << std::endl;
        return;
    }

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr)) {
        for (const auto& sentence : summary) {
            pVoice->Speak(std::wstring(sentence.begin(), sentence.end()).c_str(), SPF_IS_XML, nullptr);
        }
        pVoice->Release();
    }
    else {
        std::cerr << "TTS 엔진을 초기화할 수 없습니다." << std::endl;
    }

    CoUninitialize();
}

void summarizeFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "파일을 열 수 없습니다." << endl;
        return;
    }
    stringstream ss;
    ss << file.rdbuf();
    string utf8Str = ss.str(); // UTF-8 인코딩 문자열

    // UTF-8 문자열을 ANSI 문자열로 변환
    char* ansiStr = UTF8ToANSI(utf8Str.c_str());

    cout << ansiStr << endl; // 변환된 문자열 출력

    summarizeText(ansiStr);

    delete[] ansiStr; // 메모리 해제
}

int main() {
    int choice;
    std::cout << "이미지 입력 : 1, 텍스트 입력 : 2, 문서 입력 : 3\n";
    std::cin >> choice;
    std::cin.ignore(); // 개행 문자 무시

    if (choice == 1) {
        // 이미지 파일 경로 설정
        std::string image_path = "test.png";

        // 이미지 파일 읽어오기
        cv::Mat original_image = cv::imread(image_path);

        // 이미지를 성공적으로 읽었는지 확인
        if (original_image.empty()) {
            std::cerr << "이미지를 읽을 수 없습니다." << std::endl;
            return -1;
        }

        // 이미지 크기 조정
        cv::resize(original_image, original_image, cv::Size(), 1.0, 1.0);

        // 이미지를 그레이스케일로 변환
        cv::Mat gray_image;
        cv::cvtColor(original_image, gray_image, cv::COLOR_BGR2GRAY);

        // 이미지 이진화
        cv::Mat binary_image;
        cv::threshold(gray_image, binary_image, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        // MSER 객체 생성
        cv::Ptr<cv::MSER> mser = cv::MSER::create();

        // MSER 알고리즘을 이용하여 텍스트 영역 검출
        std::vector<std::vector<cv::Point>> regions;
        std::vector<cv::Rect> rects;
        mser->detectRegions(binary_image, regions, rects);

        // 검출된 텍스트 영역에 네모박스 그리기
        for (size_t i = 0; i < rects.size(); ++i) {
            cv::rectangle(original_image, rects[i], cv::Scalar(0, 255, 0), 2);
        }

        // 텍스트 영역이 표시된 이미지 출력
        cv::imshow("텍스트 영역 검출 결과", original_image);

        // Tesseract OCR 초기화 및 텍스트 추출
        const char* input_image = "test.png";
        const char* dataPath = "C:/Program Files/Tesseract-OCR/tessdata";

        tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
        if (api->Init(dataPath, "kor")) {
            return -1;
        }

        Pix* image = pixRead(input_image);
        api->SetImage(image);

        std::string utf_text = api->GetUTF8Text();
        std::string text = UTF8ToANSI(utf_text.c_str());

        std::cout << "OCR 결과:\n" << text << std::endl;

        summarizeText(text);

        // 메모리 해제
        api->End();
        pixDestroy(&image);

        // 키 입력 대기
        cv::waitKey(0);
    }
    else if (choice == 2) {
        // 사용자로부터 텍스트 입력받기
        std::cout << "\n사용자 텍스트 입력:\n";
        std::string userInput;
        std::getline(std::cin, userInput);

        summarizeText(userInput);
    }
    // txt 파일 읽어와서 요약
    else if (choice == 3) {
        cout << "\n파일 경로 입력:\n";
        string filePath;
        getline(cin, filePath);
        cout << endl;
        summarizeFile(filePath);
    }
    else {
        std::cerr << "잘못된 입력입니다." << std::endl;
        return -1;
    }
    return 0;
}

