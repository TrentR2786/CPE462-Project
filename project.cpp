#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;
using namespace tesseract;

int main(int argc, char* argv[]) {
  if (argc != 4) {
    cout << "Invalid number of arguments.";
    return -1;
  }

  // read image
  // Code Sources:
  // https://www.opencv-srf.com/2017/11/load-and-display-image.html, imgproc.cpp
  Mat image = imread(argv[1]);
  if (image.empty()) {
    cout << "Invalid or missing image." << endl;
    return -1;
  }

  // grayscale
  Mat image_gray;
  cvtColor(image, image_gray, COLOR_BGR2GRAY);

  // denoise
  Mat image_blur;
  blur(image_gray, image_blur, Size(3, 3));

  // threshold
  Mat image_thresh;
  threshold(image_blur, image_thresh, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);

  //Deskew and rotate image
  vector<Point> points;
  Mat_<uchar>::iterator curr = image_thresh.begin<uchar>();
  Mat_<uchar>::iterator end = image_thresh.end<uchar>();
  for (; curr != end; ++curr) {
    if (*curr) {
      points.push_back(curr.pos());
    }
  }

  RotatedRect rect = minAreaRect(Mat(points));
  Mat rot_mat = getRotationMatrix2D(rect.center, rect.angle < -45 ? rect.angle += 90 : rect.angle, 1);
  
  Mat image_rot;
  warpAffine(image_thresh, image_rot, rot_mat, image_thresh.size(), cv::INTER_CUBIC);
  bitwise_not(image_rot, image_rot);

  // initialize optical character recognition
  // Code Source:
  // https://medium.com/building-a-simple-text-correction-tool/basic-ocr-with-tesseract-and-opencv-34fae6ab3400
  TessBaseAPI* ocr = new TessBaseAPI();
  ocr->Init(NULL, "eng", OEM_LSTM_ONLY);
  ocr->SetImage(image_rot.data, image_rot.cols, image_rot.rows, 1,
                image_rot.step);
  ocr->Recognize(0);

  // initialize iterator to detect every individual word
  ResultIterator* it = ocr->GetIterator();
  PageIteratorLevel level = RIL_WORD;

  // initialize censored output image
  Mat image_censored = image_rot.clone();

  // import censored words & convert into list
  string censoredWords = argv[2];
  vector<string> censorList;
  int current_pos = 0;
  while (1) {
    int space_pos = censoredWords.find(" ", current_pos);
    if (space_pos != string::npos) {
      string censoredWord =
          censoredWords.substr(current_pos, space_pos - current_pos);
      censorList.push_back(censoredWord);
      current_pos = space_pos + 1;
    } else {
      string censoredWord = censoredWords.substr(current_pos);
      censorList.push_back(censoredWord);
      break;
    }
  }

  //censor word list debug
  /*
  for (int i = 0; i < censorList.size(); i++) {
    cout << censorList[i] << endl;
  }
  */

  // go through every word detected
  if (it != 0) {
    do {
      // detect coordinates
      const char* word = it->GetUTF8Text(level);
      int tlx, tly, brx, bry;
      it->BoundingBox(level, &tlx, &tly, &brx, &bry);

      // word information debug
      // cout << "word: '" << word << "'; Coords: (" << tlx << "," << tly << "),
      // (" << brx << ", " << bry << ") " << endl;

      // draw censor box on word if it matches w/ list
      for (int i = 0; i < censorList.size(); i++) {
        if (censorList[i].compare(word) == 0) {
          rectangle(image_censored, Point(tlx, tly), Point(brx, bry),
                    Scalar(0, 0, 0), FILLED);
        }
      }

      delete[] word;
    } while (it->Next(level));
  }

  //Display input and output images
  cv::imshow("Input", image);
  cv::imshow("Preprocessed", image_rot);
  cv::imshow("Output", image_censored);
  cv::waitKey(0);
  cv::destroyWindow("Input");
  cv::destroyWindow("Preprocessed");
  cv::destroyWindow("Output");

  ocr->End();
  imwrite(argv[3], image_censored);
  return 0;
}