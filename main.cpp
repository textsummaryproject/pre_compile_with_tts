#include <iostream>
#include <opencv2/opencv.hpp>
#include "ocr.h"
#include "summarizer.h"

using namespace std;

int main() {
    int choice;
    std::cout << "�̹��� �Է� : 1, �ؽ�Ʈ �Է� : 2, ���� �Է� : 3\n";
    std::cin >> choice;
    std::cin.ignore(); // ���� ���� ����

    if (choice == 1) {
        std::string image_path = "hmm.png";
        std::string ocr_text = performOCR(image_path);
        if (!ocr_text.empty()) {
            std::cout << "OCR ���:\n" << ocr_text << std::endl;
            summarizeText(ocr_text);
        }
    }
    else if (choice == 2) {
        std::cout << "\n����� �ؽ�Ʈ �Է�:\n";
        std::string userInput;
        std::getline(std::cin, userInput);
        summarizeText(userInput);
    }
    else if (choice == 3) {
        cout << "\n���� ��� �Է�:\n";
        string filePath;
        getline(cin, filePath);
        summarizeFile(filePath);
    }
    else {
        std::cerr << "�߸��� �Է��Դϴ�." << std::endl;
        return -1;
    }
    return 0;
}
