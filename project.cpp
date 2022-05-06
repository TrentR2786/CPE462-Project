#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {

// Code sources: https://www.opencv-srf.com/2017/11/load-and-display-image.html, imgproc.cpp (Class)
  Mat image = imread(argv[1]);

  if (image.empty()) {
    cout << "Could not open or find the image" << endl;
    return -1;
  }

  imshow("Input Image", image);
  waitKey(0);
  destroyWindow("Input Image");
// ---------------------------------------------------


}
