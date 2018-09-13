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
	std::string mDebugPath;
	cv::Size mSeamImgSize;
	cv::Size mSourceImgSize;
	float mWarpedImageScale = 1;
	double mWorkScale = 1;

	const bool mUseGPU;

	std::vector<cv::UMat> mSeamMasks;

	cv::Ptr<cv::WarperCreator> mWarperCreator;
	cv::Ptr<cv::detail::ExposureCompensator> mExposureCompensator;
		
	int mExposureCompensatorType;

	void prepare_exposure_compensator(const std::vector<cv::Point>& corners,
									  std::vector<cv::UMat>& images_warped,
									  const std::vector<cv::UMat>& masks_warped);

	void find_seams(const std::vector<cv::Point>& corners,
				    const std::vector<cv::UMat>& images,
				    std::vector<cv::UMat>& masks);

	void prepare_composition(const std::vector<cv::UMat>& images,
							 std::vector<cv::detail::CameraParams>& camera_params);
};

#endif /* PANORAMAMAKER_HPP*/