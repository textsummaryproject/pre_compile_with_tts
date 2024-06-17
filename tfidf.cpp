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

// UTF-8 ���ڿ��� ANSI ���ڿ��� ��ȯ�ϴ� �Լ�
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

// �ܾ �ҹ��ڷ� ��ȯ�ϴ� �Լ�
std::string toLowerCase(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// ���ڿ��� �ܾ�� �����ϴ� �Լ�
std::vector<std::string> split(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// TF-IDF ��� �Լ�
std::map<std::string, double> computeTFIDF(const std::vector<std::string>& doc, const std::vector<std::vector<std::string>>& corpus) {
    std::map<std::string, double> tfidf;
    std::map<std::string, int> termFrequency;
    std::map<std::string, int> documentFrequency;
    int totalTerms = doc.size();

    // TF ���
    for (const std::string& word : doc) {
        termFrequency[word]++;
    }

    // DF ���
    for (const std::vector<std::string>& document : corpus) {
        std::set<std::string> uniqueWords(document.begin(), document.end());
        for (const std::string& word : uniqueWords) {
            documentFrequency[word]++;
        }
    }

    // TF-IDF ���
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

    // ������ �ܾ�� ����
    std::vector<std::vector<std::string>> corpus;
    for (const std::string& sentence : sentences) {
        corpus.push_back(split(toLowerCase(sentence)));
    }

    // �� ������ TF-IDF ���
    std::vector<std::pair<std::string, double>> sentenceScores;
    for (const std::vector<std::string>& doc : corpus) {
        std::map<std::string, double> tfidf = computeTFIDF(doc, corpus);
        double score = 0.0;
        for (const auto& pair : tfidf) {
            score += pair.second;
        }
        sentenceScores.push_back(std::make_pair(sentences[&doc - &corpus[0]], score));
    }

    // TF-IDF ������ ���� ������ ���� ����
    std::sort(sentenceScores.begin(), sentenceScores.end(), [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
        return b.second > a.second;
        });

    // ��� ��� ��� (���� 3����)
    std::cout << "��� ���:\n";
    std::vector<std::string> summary;
    for (size_t i = 0; i < std::min<size_t>(sentenceScores.size(), size_t(3)); ++i) {
        std::cout << sentenceScores[i].first << "\n";
        summary.push_back(sentenceScores[i].first);
    }

    // TTS�� ���� ���� �б�
    ISpVoice* pVoice = nullptr;

    if (FAILED(::CoInitialize(nullptr))) {
        std::cerr << "COM ���̺귯���� �ʱ�ȭ�� �� �����ϴ�." << std::endl;
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
        std::cerr << "TTS ������ �ʱ�ȭ�� �� �����ϴ�." << std::endl;
    }

    CoUninitialize();
}

void summarizeFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "������ �� �� �����ϴ�." << endl;
        return;
    }
    stringstream ss;
    ss << file.rdbuf();
    string utf8Str = ss.str(); // UTF-8 ���ڵ� ���ڿ�

    // UTF-8 ���ڿ��� ANSI ���ڿ��� ��ȯ
    char* ansiStr = UTF8ToANSI(utf8Str.c_str());

    cout << ansiStr << endl; // ��ȯ�� ���ڿ� ���

    summarizeText(ansiStr);

    delete[] ansiStr; // �޸� ����
}

int main() {
    int choice;
    std::cout << "�̹��� �Է� : 1, �ؽ�Ʈ �Է� : 2, ���� �Է� : 3\n";
    std::cin >> choice;
    std::cin.ignore(); // ���� ���� ����

    if (choice == 1) {
        // �̹��� ���� ��� ����
        std::string image_path = "test.png";

        // �̹��� ���� �о����
        cv::Mat original_image = cv::imread(image_path);

        // �̹����� ���������� �о����� Ȯ��
        if (original_image.empty()) {
            std::cerr << "�̹����� ���� �� �����ϴ�." << std::endl;
            return -1;
        }

        // �̹��� ũ�� ����
        cv::resize(original_image, original_image, cv::Size(), 1.0, 1.0);

        // �̹����� �׷��̽����Ϸ� ��ȯ
        cv::Mat gray_image;
        cv::cvtColor(original_image, gray_image, cv::COLOR_BGR2GRAY);

        // �̹��� ����ȭ
        cv::Mat binary_image;
        cv::threshold(gray_image, binary_image, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        // MSER ��ü ����
        cv::Ptr<cv::MSER> mser = cv::MSER::create();

        // MSER �˰����� �̿��Ͽ� �ؽ�Ʈ ���� ����
        std::vector<std::vector<cv::Point>> regions;
        std::vector<cv::Rect> rects;
        mser->detectRegions(binary_image, regions, rects);

        // ����� �ؽ�Ʈ ������ �׸�ڽ� �׸���
        for (size_t i = 0; i < rects.size(); ++i) {
            cv::rectangle(original_image, rects[i], cv::Scalar(0, 255, 0), 2);
        }

        // �ؽ�Ʈ ������ ǥ�õ� �̹��� ���
        cv::imshow("�ؽ�Ʈ ���� ���� ���", original_image);

        // Tesseract OCR �ʱ�ȭ �� �ؽ�Ʈ ����
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

        std::cout << "OCR ���:\n" << text << std::endl;

        summarizeText(text);

        // �޸� ����
        api->End();
        pixDestroy(&image);

        // Ű �Է� ���
        cv::waitKey(0);
    }
    else if (choice == 2) {
        // ����ڷκ��� �ؽ�Ʈ �Է¹ޱ�
        std::cout << "\n����� �ؽ�Ʈ �Է�:\n";
        std::string userInput;
        std::getline(std::cin, userInput);

        summarizeText(userInput);
    }
    // txt ���� �о�ͼ� ���
    else if (choice == 3) {
        cout << "\n���� ��� �Է�:\n";
        string filePath;
        getline(cin, filePath);
        cout << endl;
        summarizeFile(filePath);
    }
    else {
        std::cerr << "�߸��� �Է��Դϴ�." << std::endl;
        return -1;
    }
    return 0;
}

