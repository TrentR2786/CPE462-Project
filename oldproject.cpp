#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>
#include <iostream>
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

  Mat image_dilate;
  Mat dilatekernel = getStructuringElement(MORPH_RECT, Size(7,7));
  morphologyEx(image_thresh, image_dilate, MORPH_CLOSE, dilatekernel);
  imshow("morph", image_dilate);
  waitKey(0);
  destroyWindow("morph");

  //find contours
  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(image_dilate, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

  //find+draw rectangles in red
  Mat image_rect;
  cvtColor(image_thresh, image_rect, COLOR_GRAY2BGR);
  vector<Rect> boundRect(contours.size());

  for (size_t i = 0; i < contours.size(); i++) {
    boundRect[i] = boundingRect(contours[i]);
    rectangle(image_rect, boundRect[i].tl(), boundRect[i].br(), Scalar(0,0,255));
  }

  /*
  // TODO: merge nearby rectangles to create large rectangular blocks with words
  for (size_t i = 0; i < boundRect.size(); i++) {
    for (size_t j = 0; j < boundRect.size(); j++) {
        if (boundRect[i] != boundRect[j]) {
          if (abs(boundRect[i].x - boundRect[j].br().x) < 10 || abs(boundRect[j].x - boundRect[i].br().x) < 10) {
            if (abs(boundRect[i].y - boundRect[j].y) < 1)
              boundRect[j] |= boundRect[i];
              boundRect.pop_back();
          }
      }
    }
  }
  */
  
  //display rects
  imshow("Rect Image1", image_rect);
  waitKey(0);
  destroyWindow("Rect Image1");
  
  /*
  // test for turning individual boundrects into images for ocr input
  for (size_t i = 0; i < boundRect.size(); i++) {
    Mat roitest(image_thresh, Rect(boundRect[i]));
    imshow("roitest", roitest);
    waitKey(0);
    destroyWindow("roitest");
  }
  */

  // TODO: crosscheck w/ words.txt to see whether or not to censor

  //draw black boxes on contours (test for censoring)
  Mat image_censored = image.clone();
  for (size_t i = 0; i < boundRect.size(); i++) {
    rectangle(image_censored, boundRect[i].tl(), boundRect[i].br(), Scalar(0,0,0), FILLED);
  }

  //display censor
  imshow("Censored Image", image_censored);
  waitKey(0);
  destroyWindow("Censored Image");

  /*
  tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
  ocr->SetImage(image_thresh.data, image_thresh.cols, image_thresh.rows, 1, image_thresh.step);

  string text = string(ocr->GetUTF8Text());
  cout << text;
  ocr->End();
  return 0;
  */

  Mat roitest = image_thresh.clone();

  tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr->SetImage(image_thresh.data, image_thresh.cols, image_thresh.rows, 1, image_thresh.step);
  ocr->Recognize(0);
  tesseract::ResultIterator* it = ocr->GetIterator();
  tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

  if (it != 0) {
    do {
      const char* word = it->GetUTF8Text(level);
      int tlx, tly, brx, bry;
      it->BoundingBox(level, &tlx, &tly, &brx, &bry);
      cout << "word: '" << word << "'; Coords: (" << tlx << "," << tly << "), (" << brx << "," << bry << ")" << endl;
      rectangle(roitest, Point(tlx,tly), Point(brx, bry), Scalar(255,255,0));
      delete[] word;
    } while (it->Next(level));
  }

  imshow("roitest", roitest);
  waitKey(0);
  destroyWindow("roitest");

  ocr->End();
  return 0;
}