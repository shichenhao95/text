#include "opencv2/opencv.hpp"

int main(int argc, char **argv) {
	// Create a named window with the name of the file
	cv::namedWindow(argv[1], cv::WINDOW_GUI_EXPANDED);

	// Load the image from the given filename
	cv::Mat img = cv::imread(argv[1]);

	// Show the image in the named window
	cv::imshow(argv[1], img);

	// Idle until the user hits the Esc key
	while (true) {
		if (cv::waitKey(100) == 27) break;
	}

	// Clean up and don't be piggiers
	cv::destroyWindow(argv[1]);

	exit(0);
}