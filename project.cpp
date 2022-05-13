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
  threshold(image_blur, image_thresh, 0, 255, THRESH_BINARY + THRESH_OTSU);

  //initialize optical character recognition
  tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr->SetImage(image_thresh.data, image_thresh.cols, image_thresh.rows, 1, image_thresh.step);
  ocr->Recognize(0);
  
  //initialize iterator to detect every individual word
  tesseract::ResultIterator* it = ocr->GetIterator();
  tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

  //initialize censored output image
  Mat image_censored = image.clone();

  //go through every word detected
  if (it != 0) {
    do {
      //detect coordinates
      const char* word = it->GetUTF8Text(level);
      int tlx, tly, brx, bry;
      it->BoundingBox(level, &tlx, &tly, &brx, &bry);

      //word information
      cout << "word: '" << word << "'; Coords: (" << tlx << "," << tly << "), (" << brx << "," << bry << ")" << endl;

      // TODO: crosscheck w/ words.txt to see whether or not to censor

      //draw censor box on word
      rectangle(image_censored, Point(tlx,tly), Point(brx, bry), Scalar(0,0,0), FILLED);

      delete[] word;
    } while (it->Next(level));
  }

  //display input and output
  imshow("Input", image);
  imshow("Output", image_censored);
  waitKey(0);
  destroyWindow("Input");
  destroyWindow("Output");

  ocr->End();
  return 0;
}