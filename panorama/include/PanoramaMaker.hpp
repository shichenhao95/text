#ifndef PANORAMAMAKER_HPP
#define PANORAMAMAKER_HPP

class PanoramaMaker {
public:
	PanoramaMaker(const bool useGPU = true);
	~PanoramaMaker();

	bool analyze(const std::vector<cv::UMat>& images,
				 const float work_megapix = 0.6,
				 const float seam_megapix = 0.1);

private:
	bool mDebugMode;
	cv::Size mSourceImgSize;
	double mWorkScale = 1;

	const bool mUseGPU;

	cv::Ptr<cv::WarperCreator> mWarperCreator;

	int mExposureCompensatorType;
};

#endif /* PANORAMAMAKER_HPP*/