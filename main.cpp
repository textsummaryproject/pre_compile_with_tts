#include <iostream>
#include <opencv2/opencv.hpp>
#include "ocr.h"
#include "summarizer.h"

using namespace std;

int main() {
    int choice;
    std::cout << "이미지 입력 : 1, 텍스트 입력 : 2, 문서 입력 : 3\n";
    std::cin >> choice;
    std::cin.ignore(); // 개행 문자 무시

    if (choice == 1) {
        std::string image_path = "hmm.png";
        std::string ocr_text = performOCR(image_path);
        if (!ocr_text.empty()) {
            std::cout << "OCR 결과:\n" << ocr_text << std::endl;
            summarizeText(ocr_text);
        }
    }
    else if (choice == 2) {
        std::cout << "\n사용자 텍스트 입력:\n";
        std::string userInput;
        std::getline(std::cin, userInput);
        summarizeText(userInput);
    }
    else if (choice == 3) {
        cout << "\n파일 경로 입력:\n";
        string filePath;
        getline(cin, filePath);
        summarizeFile(filePath);
    }
    else {
        std::cerr << "잘못된 입력입니다." << std::endl;
        return -1;
    }
    return 0;
}
