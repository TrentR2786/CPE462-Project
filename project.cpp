#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
  //read image
  Mat image = imread(argv[1]);
  if (image.empty()) {
    cout << "Invalid image" << endl;
    return 1;
  }

  //grayscale
  Mat image_gray;
  cvtColor(image, image_gray, COLOR_BGR2GRAY);

  //denoise
  Mat image_blur;
  blur(image_gray, image_blur, Size(3,3));

  //threshold
  Mat image_thresh;
  threshold(image_blur, image_thresh, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);
  
  //find contours
  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(image_thresh, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

  //find+draw rectangles in red
  Mat image_rect;
  cvtColor(image_thresh, image_rect, COLOR_GRAY2BGR);
  vector<Rect> boundRect(contours.size());

  for (size_t i = 0; i < contours.size(); i++) {
    boundRect[i] = boundingRect(contours[i]);
    rectangle(image_rect, boundRect[i].tl(), boundRect[i].br(), Scalar(0,0,255));
  }

  //display rects
  imshow("Rect Image", image_rect);
  waitKey(0);
  destroyWindow("Rect Image");

  //draw black boxes on contours (test for censoring)
  Mat image_censored = image.clone();
  for (size_t i = 0; i < contours.size(); i++) {
    rectangle(image_censored, boundRect[i].tl(), boundRect[i].br(), Scalar(0,0,0), FILLED);
  }

  //display censor
  imshow("Censored Image", image_censored);
  waitKey(0);
  destroyWindow("Censored Image");

  /*****************************************/
  // Code source:
  // https://medium.com/building-a-simple-text-correction-tool/basic-ocr-with-tesseract-and-opencv-34fae6ab3400
  tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr->SetPageSegMode(tesseract::PSM_AUTO);
  ocr->SetImage(image_thresh.data, image_thresh.cols, image_thresh.rows, 1, image_thresh.step);

  string text = string(ocr->GetUTF8Text());
  cout << text;
  ocr->End();
  return 0;
  /*****************************************/
}