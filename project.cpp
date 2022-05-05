#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
  Mat image = imread(argv[1]);

  if (image.empty()) {
    cout << "Could not open or find the image" << endl;
    return -1;
  }
}