#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
  /*****************************************
  Code sources:
  https://www.opencv-srf.com/2017/11/load-and-display-image.html, imgproc.cpp */

  Mat image = imread(argv[1]);

  if (image.empty()) {
    cout << "Could not open or find the image" << endl;
    return -1;
  }

  imshow("Input Image", image);
  waitKey(0);
  destroyWindow("Input Image");

  Mat_<uchar> image_gray(image.rows, image.cols);
  cvtColor(image, image_gray, COLOR_BGR2GRAY);

  /*****************************************/

  /*****************************************
  Code source:
  https://medium.com/building-a-simple-text-correction-tool/basic-ocr-with-tesseract-and-opencv-34fae6ab3400
*/

  tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr->SetPageSegMode(tesseract::PSM_AUTO);
  ocr->SetImage(image_gray.data, image.cols, image.rows, 1, image_gray.step);

  string text = string(ocr->GetUTF8Text());
  cout << text;
  ocr->End();

  /*****************************************/
}
