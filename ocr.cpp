#include "ocr.h"
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include "utils.h"

std::string performOCR(const std::string& imagePath) {
    cv::Mat original_image = cv::imread(imagePath);

    if (original_image.empty()) {
        std::cerr << "이미지를 읽을 수 없습니다." << std::endl;
        return "";
    }

    cv::resize(original_image, original_image, cv::Size(), 1.0, 1.0);
    cv::Mat gray_image;
    cv::cvtColor(original_image, gray_image, cv::COLOR_BGR2GRAY);
    cv::Mat binary_image;
    cv::threshold(gray_image, binary_image, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    cv::Ptr<cv::MSER> mser = cv::MSER::create();
    std::vector<std::vector<cv::Point>> regions;
    std::vector<cv::Rect> rects;
    mser->detectRegions(binary_image, regions, rects);

    for (size_t i = 0; i < rects.size(); ++i) {
        cv::rectangle(original_image, rects[i], cv::Scalar(0, 255, 0), 2);
    }

    cv::imshow("텍스트 영역 검출 결과", original_image);

    const char* input_image = imagePath.c_str();
    const char* dataPath = "C:/Program Files/Tesseract-OCR/tessdata";
    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    if (api->Init(dataPath, "kor")) {
        return "";
    }

    Pix* image = pixRead(input_image);
    api->SetImage(image);

    std::string utf_text = api->GetUTF8Text();
    std::string text = UTF8ToANSI(utf_text.c_str());

    api->End();
    pixDestroy(&image);

    cv::waitKey(0);

    return text;
}
