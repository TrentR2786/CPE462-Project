#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;
using namespace tesseract;

string normalize(string word) {
  // Make whole word lowercase, ignore case sensitivity
  transform(word.begin(), word.end(), word.begin(), ::tolower);

  // Remove all non-alphanumeric characters from word
  for (auto it = word.begin(); it != word.end(); it++) {
    if (!isalnum(word.at(it - word.begin()))) {
      word.erase(it);
      it--;
    }
  }

  return word;
}

int main(int argc, char* argv[]) {
  // Check for correct number of arguments
  if (argc != 4) {
    cout << "Usage: " << argv[0] << " <input filename> <list of censored words (in one quote, words separated by spaces)> <output filename>";
    return -1;
  }

  // Read image
  Mat image = imread(argv[1]);
  if (image.empty()) {
    cout << "Invalid or missing image." << endl;
    return -1;
  }

  // Convert image to grayscale
  Mat image_gray;
  cvtColor(image, image_gray, COLOR_BGR2GRAY);

  // Rescale image for better legibility
  Mat image_scaled;
  resize(image_gray, image_scaled, Size(0,0), 1.2, 1.2, INTER_CUBIC);

  // Gaussian blur the image to remove noise
  Mat image_blur;
  GaussianBlur(image_scaled, image_blur, Size(0,0), 33, 33);
  divide(image_scaled, image_blur, image_blur, 255);

  // Binarization mask to convert text to 255 and everything else to 0
  Mat image_thresh;
  threshold(image_blur, image_thresh, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);

  // Modified deskew code based on original code from
  // http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/
  // Find all points in skewed image
  vector<Point> points;
  for (Mat_<uchar>::iterator it = image_thresh.begin<uchar>(); it != image_thresh.end<uchar>(); ++it) {
    if (*it) {
      points.push_back(it.pos());
    }
  }

  // Create largest bounding box and rotation matrix for skewed image
  RotatedRect rect = minAreaRect(Mat(points));
  Mat rot_mat = getRotationMatrix2D(rect.center, rect.angle < -45 ? rect.angle += 90 : rect.angle, 1);

  // Deskew image
  Mat image_rot;
  warpAffine(image_thresh, image_rot, rot_mat, image_thresh.size(), INTER_CUBIC);

  // Invert binarization mask so Tesseract can read it more accurately
  bitwise_not(image_rot, image_rot);

  // Modified Tesseract API code based on original examples from
  // https://medium.com/building-a-simple-text-correction-tool/basic-ocr-with-tesseract-and-opencv-34fae6ab3400
  // https://tesseract-ocr.github.io/tessdoc/APIExample.html
  // Initialize optical character recognition library
  TessBaseAPI* ocr = new TessBaseAPI();
  ocr->Init(NULL, "eng", OEM_LSTM_ONLY);
  ocr->SetImage(image_rot.data, image_rot.cols, image_rot.rows, 1, image_rot.step);
  ocr->Recognize(0);

  // Initialize iterator to detect each individual word
  ResultIterator* it = ocr->GetIterator();
  PageIteratorLevel level = RIL_WORD;

  // Initialize censored output image
  Mat image_censored = image_rot.clone();

  // Import censored words & convert into list
  string censoredWords = argv[2];
  vector<string> censorList;
  int current_pos = 0;
  while (1) {
    int space_pos = censoredWords.find(" ", current_pos);
    if (space_pos != string::npos) {
      string censoredWord = censoredWords.substr(current_pos, space_pos - current_pos);
      censoredWord = normalize(censoredWord);
      censorList.push_back(censoredWord);
      current_pos = space_pos + 1;
    } else {
      string censoredWord = censoredWords.substr(current_pos);
      censoredWord = normalize(censoredWord);
      censorList.push_back(censoredWord);
      break;
    }
  }

  // Print out censored words (DEBUG)
  /*
  for (int i = 0; i < censorList.size(); i++) {
    cout << censorList[i] << endl;
  }
  */

  // Go through every word detected
  if (it != 0) {
    do {
      // Detect bounding box coordinates from image using Tesseract
      string word = string(it->GetUTF8Text(level));
      int tlx, tly, brx, bry;
      it->BoundingBox(level, &tlx, &tly, &brx, &bry);

      // Print coordinate information (DEBUG)
      //  cout << "word: '" << word << "'; Coords: (" << tlx << "," << tly << "), (" << brx << ", " << bry << ") " << endl;

      word = normalize(word);

      // Draw censor box on word if it matches with list
      for (int i = 0; i < censorList.size(); i++) {
        if (censorList[i].compare(word) == 0) {
          rectangle(image_censored, Point(tlx, tly), Point(brx, bry), Scalar(0, 0, 0), FILLED);
        }
      }
    } while (it->Next(level));
  }

  // Display input and output images
  imshow("Input", image);
  imshow("Preprocessed", image_rot);
  imshow("Output", image_censored);
  waitKey(0);
  destroyWindow("Input");
  destroyWindow("Preprocessed");
  destroyWindow("Output");

  ocr->End();
  // Save output image
  imwrite(argv[3], image_censored);
  return 0;
}