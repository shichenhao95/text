#include <sstream>
#include "devutil.hpp"

void saveImages(const std::string& basename, const std::vector<cv::Mat>& images) {
	for (auto i = 0; i < images.size(); i++) {
		std::stringstream ss;
		ss << basename << i << ".png";

		cv::imwrite(ss.str(), images[i]);
	}
}

void saveImages(const std::string& basename, const std::vector<cv::UMat>& images) {
	for (auto i = 0; i < images.size(); i++) {
		std::stringstream ss;
		ss << basename << i << ".png";

		cv::imwrite(ss.str(), images[i].getMat(cv::ACCESS_READ));
	}
}