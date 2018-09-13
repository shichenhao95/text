#ifndef DEVUTIL_H
#define DEVUTIL_H

#include<string>
#include<vector>

void saveImages(const std::string& basename, const std::vector<cv::Mat>& images);
void saveImages(const std::string& basename, const std::vector<cv::UMat>& images);

#endif /* DEVUTIL_H */