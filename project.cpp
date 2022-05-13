#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
  /*****************************************/
  // Code sources:
  // https://www.opencv-srf.com/2017/11/load-and-display-image.html, imgproc.cpp

  Mat image = imread(argv[1]);

  if (image.empty()) {
    cout << "Could not open or find the image" << endl;
    return -1;
  }

  /*****************************************/

  //grayscale
  Mat image_gray;
  cvtColor(image, image_gray, COLOR_BGR2GRAY);

  //denoise
  Mat image_blur;
  blur(image_gray, image_blur, Size(3,3));

  //threshold
  Mat image_thresh;
  threshold(image_blur, image_thresh, 0, 255, THRESH_BINARY + THRESH_OTSU);
  
  //find contours
  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(image_thresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

  //draw contours in green
  Mat image_contours;
  cvtColor(image_thresh, image_contours, COLOR_GRAY2BGR);
  Mat image_rect = image_contours.clone();

  for (size_t i = 0; i < contours.size(); i++) {
    drawContours(image_contours, contours, i, Scalar(0,255,0));
  }

  //find+draw rectangles in red
  vector<Rect> boundRect(contours.size());
  for (size_t i = 0; i < contours.size(); i++) {
    boundRect[i] = boundingRect(contours[i]);
    rectangle(image_rect, boundRect[i].tl(), boundRect[i].br(), Scalar(0,0,255));
  }

  //display
  imshow("Rect Image", image_rect);
  waitKey(0);
  destroyWindow("Rect Image");

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
  
  /*****************************************/
}