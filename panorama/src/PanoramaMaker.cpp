#include "PanoramaMaker.hpp"

static float calcWarpedImageScale(const std::vector<cv::detail::CameraParams>& cameras) {
	std::vector<double> focals;
	for (int i = 0; i < cameras.size(); i++) {
		focals.push_back(cameras[i].focal);
	}

	std::sort(focals.begin(), focals.end());

	if (focals.size() % 2 == 1)
		return static_cast<float>(focals[focals.size() / 2]);

	return static_cast<float>(focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) * 0.5f;
}

static void doWaveCorrect(std::vector<cv::detail::CameraParams>& cameras, const cv::detail::WaveCorrectKind wave_correct_kind = cv::detail::WAVE_CORRECT_HORIZ) {
	std::vector<cv::Mat> rmats;
	for (size_t i = 0; i < cameras.size(); i++)
		rmats.push_back(cameras[i].R.clone());

	// void waveCorrect( std::vector<Mat> &rmats, 
	// 					 WaveCorrectKind kind);
	// Function: Tries to make panorama more horizontal (or vertical).
	// Parameters: 
	//		rmats -- Camera rotation matrices.
	//		kind -- Correction kind, see detail::WaveCorrectKind.
	cv::detail::waveCorrect(rmats, wave_correct_kind);

	for (size_t i = 0; i < cameras.size(); i++)
		cameras[i].R = rmats[i];
}

PanoramaMaker::PanoramaMaker(const bool useGPU)
	: mUseGPU(useGPU), mDebugMode(false) {
	if (useGPU) {
		mWarperCreator = new cv::CylindricalWarperGPU();
	}
	else {
		mWarperCreator = new cv::CylindricalWarper();
	}

	mExposureCompensatorType = new cv::detail::ExposureCompensator::NO;
}

PanoramaMaker::~PanoramaMaker()
{
}

bool PanaramaMaker::analyze(const std::vector<cv::UMat>& images,
							const float work_megapix,
							const float seam_megapix) {
	const auto num_images = images.size();
	if (num_images < 2)
	{
		std::cout << "Less than two images!" << std::endl;
		return false;
	}

	mSourceImgSize = images[0].size();

	// cv::detail::ImageFeatures 
	// Function: Structure containing image keypoints and descriptors.
	std::vector<cv::detail::ImageFeatures> image_features(num_images);
	// cv::detail::MatchesInfo
	// Function: Structure containing information about matches between two images. It's assumed that there is a transformation between those images. Transformation may be
	//			 homography or affine transformation based on selected matcher.
	std::vector<cv::detail::MatchesInfo> pairwise_matches(num_images * num_images);

	mWorkScale = work_megapix < 0
		? 1.0
		: std::min(1.0, sqrt(work_megapix * 1e6 / mSourceImgSize.area()));

	std::cout << "Finding features\n"
		<< " work_megapix:" << work_megapix << "\n"
		<< " work_scale:" << mWorkScale << "\n"
		<< std::endl;

	{
		cv::detail::SurfFeaturesFinder feature_finder(300.0, 3, 4, 3, 4);

		cv::Mat tmping;
		std::vector<cv::Mat> painted_features(num_images);

		for (auto i = 0; i < num_images; i++) {
			if (work_megapix < 0) {
				std::cout << "No resizing" << std::endl;

				const int width = images[i].cols;
				const int width_part = width / 3;
				const int height = images[i].rows;
				const int right_x = width - width_part;

				std::vector<cv::Rect> rois;
				rois.push_back(cv::Rect(0, 0, width_part, height));
				rois.puch_back(cv::Rect(right_x, 0, width_part, height));

				// void operator ()(InputArray image, ImageFeatures &features, const std::vector<cv::Rect> &rois);
				// Function: Finds features in the given image.
				// Parameters:
				//	image -- source image.
				//	features --  found features.
				//	rois --  rois Regions of interest.
				feature_finder(images[i], image_features[i], rois);

				// void cv::drawKeypoints( InputArray image,
				//						   const std::vector< KeyPoint > & keypoints,
				//						   InputOutputArray outImage,
				//						   const Scalar & color = Scalar::all(-1),
				//						   int flags = DrawMatchesFlags::DEFAULT)
				// Function: Draw keypoints
				// Parameters:
				//		image -- Source image.
				//		keypoints -- Keypoints from the source image.
				//	    outImage -- Output image.Its content depends on the flags value defining what is drawn in the output image.See possible flags bit values below.
				//		color -- Color of keypoints.
				//		flags -- Flags setting drawing features.Possible flags bit values are defined by DrawMatchesFlags.See details above in drawMatches .
				cv::drawKeypoints(images[i], image_features[i].keypoints, painted_features[i]);
			}
			else {
				// void cv::resize( cv::InputArray src,
				//					cv::OutputArray dst,
				//					cv::Size dsize,
				//					double fx = 0,
				//					doubke fy = 0,
				//					int interpolation = CV::INTER_LINEAR)	
				// Function: Resizes an image.The function resize resizes the image src down to or up to the specified size. Note that the initial dst type or size are not taken into account. 
				//			 Instead, the size and type are derived from the src,dsize,fx, and fy. If you want to resize src so that it fits the pre-created dst, you may call the function as follows:
				//				// explicitly specify dsize=dst.size(); fx and fy will be computed from that.
				//				resize(src, dst, dst.size(), 0, 0, interpolation);
				//
				//			 If you want to decimate the image by factor of 2 in each direction, you can call the function this way: 
				//				// specify fx and fy and let the function compute the destination image size.
				//				resize(src, dst, Size(), 0.5, 0.5, interpolation);
				// Parameters:
				//		src -- Input image
				//		dst -- Result image
				//		dsize -- New size
				//		fx -- x-rescale
				//		fy -- y-rescale
				//		interpolation -- interpolation method
				cv::resize(images[i], tmping, cv::Size(), mWorkScale, mWorkScale, cv::INTER_LINEAR_EXACT);
				std::cout << "Resizing to " << tmping.size() << std::endl;

				const int width = tmping.cols;
				const int width_part = width / 3;
				const int height = tmping.rows;
				const int right_x = width - width_part;

				std::vector<cv::Rect> rois;
				rois.push_back(cv::Rect(0, 0, width_part, height));
				rois.puch_back(cv::Rect(right_x, 0, width_part, height));

				feature_finder(tmping, image_features[i], rois);

				cv::drawKeypoints(tmping, image_features[i].keypoints, painted_features[i]);
			}

			// Save the current image index in case we reorder or something
			image_features[i].img_idx = i;
		}
		if (mDebugMode)
			saveImages(mDebugPath + "/features-", painted_features);
	}

	std::cout << "Matching features" << std::endl;
	{
		// BestOf2NearestMatcher( bool try_use_gpu = false, 
		//						  float match_conf = 0.3f, 
		//						  int num_matches_thresh1 = 6,
		//						  int num_matches_thresh2 = 6);
		// Function: Constructs a "best of 2 nearest" matcher.
		// Parameters:
		//		try_use_gpu -- Should try to use GPU or not
		//		match_conf -- Match distances ration threshold
		//		num_matches_thresh1 -- Minimum number of matches required for the 2D projective transform
		//							   estimation used in the inliers classification step
		//		num_matches_thresh2 -- Minimum number of matches required for the 2D projective transform
		//							   re-estimation on inliers
		cv::detail::BestOf2NearestMatcher feature_matcher(mUseGPU, (float)0.3, 6, 6);
		cv::Mat image_links(static_cast<int>(num_images), static_cast<int>(num_images), CV_8U, cv::Scalar(0));

		for (auto i = 0; i < num_images - 1; i++) {
			images_link.at<char>(i, i + 1) = 1;
		}

		image_links.at<char>(static_cast<int>(num_images) - 1, 0) = 1;
		feature_matcher(image_features, pairwise_matches, image_links.getUMat(cv::ACCESS_READ));
	}

	std::cout << "Adjusting cameras" << std::endl;

	std::vector<cv::detail::CameraParams> camera_params;
	{
		const double confidence_threshold = 0.8;
		cv::detail::HomegraphyBasedEstimator estimator;
		// Function: Implementation of the camera parameters refinement algorithm which minimizes sum of the distances
		//	         between the rays passing through the camera center and a feature. It can estimate focal length.It ignores the refinement mask for now.
		cv::detail::BundleAdjusterRay bundle_asjuster;
		bundle_adjuster.setConfThresh(confidence_threshold);

		// Estimates camera parameters.
		if (!estimator(image_features, pairwise_matches, camera_params)) {
			std::cout << "Fail to estimate camera parameters!" << std::endl;
			return false;
		}

		for (int i = 0; i < camera_params.size(); i++) {
			cv::Mat R;
			// void convertTo( OutputArray m, 
			//				   int rtype, 
			//				   double alpha = 1, 
			//				   double beta = 0) const;
			// Function: Converts an array to another data type with optional scaling.
			//           The method converts source pixel values to the target data type.saturate_cast\<\> is applied at
			//			 the end to avoid possible overflows :
			//				\f[m(x, y) = saturate \_ cast<rType>(\alpha(*this)(x, y) + \beta)\f]
			// Parameters: 
			//		m -- output matrix; if it does not have a proper size or type before the operation, it is reallocated.
			//		rtype -- desired output matrix type or , rather, the depth since the number of channels are the same as the input has; 
			//				 if rtype is negative, the output matrix will have the same type as the input.
			//		alpha -- optional scale factor.
			//		beta -- optional delta added to the scaled values.
			camera_params[i].R.convertTo(R, CV_32F);
			camera_params[i].R = R;
			std::cout << "Camera " << i << ": " << camera_params[i] << std::endl;
		}

		if (!bundle_adjuster(image_features, pairwise_matches, camera_params)) {
			std::cout << "Fail to adjust camera parameters!" << std::endl;
			return false;
		}

		mWarpedImageScale = calcWarpedImageScale(camera_params);

		doWaveCorrect(camera_params);
	}

	std::cout << "Finding seams" << std::endl;
	{
		const float seam_scale = std::min(1.0f, static_cast<float>(sqrt(seam_megapix * 1e6f / mSourceImaSize.area())));
		const float seam_work_aspect = seam_scale / static_cast<float>(mWorkScale);

		std::cout << "	work_scale:" << mWorkScale << std::endl
				  << "	mWarpedImageSize:" << mWarpedImageScale << std::endl
				  << "	seam_work_aspect:" << seam_work_aspect << std::endl
				  << "	result:" << (mWarpedImageScale * seam_work_aspect) << std::endl;

		cv::Ptr<cv::detail::RotationWarper> seam_warper = mWarperCreator->create(mWarpedImageScale * seam_work_aspect);

		std::vector<cv::UMat> seam_est_images(num_images);
		std::vector<cv::Point> seam_corners(num_images);
		std::vector<cv::UMat> seam_warped(num_images);
		std::vector<cv::UMat> seam_masks_warped(num_images);
		std::vector<cv::UMat> tmp_masks(num_images);

		for (int i = 0; i < num_images; i++) {
			cv::Size(images[i], seam_est_images[i], cv::Size(), seam_scale, seam_scale, cv::INTER_LINEAR_EXACT);
		}

		for (int i = 0; i < num_images; i++) {
			tmp_masks[i].create(seam_est_images[i].size(), CV_8U);
			tmp_masks[i].setTo(cv::Scalar::all(255));
		}

		mSeamImgSize = seam_est_images[0].size();

		for (auto i = 0; i < num_images; i++) {
			cv::Mat_<float> K;
			camera_params[i].K().convertTo(K, CV_32F);
			K(0, 0) *= seam_work_aspect;
			K(0, 2) *= seam_work_aspect;
			K(1, 1) *= seam_work_aspect;
			K(1, 2) *= seam_work_aspect;

			//	Point warp( InputArray src, 
			//			    InputArray K, 
			//			    InputArray R, 
			//              int interp_mode, 
			//				int border_mode,
			//				OutputArray dst);
			//	Projects the image.
			//  Parameters:
			//		src -- Source image
			//		K -- Camera intrinsic parameters
			//		R -- Camera rotation matrix
			//		interp_mode -- Interpolation mode
			//		border_mode -- Border extrapolation mode
			//		dst -- Projected image
			//	Return: Project image top - left corner
			seam_corners[i] = seam_warper->warp(seam_est_images[i], K, camera_params[i].R, cv::INTER_LINER, cv::BORDER_REFLECT, seams_warped[i]);
			seam_warper->warp(tmp_mask[i], K, camera_params[i].R, cv::INTER_NEARST, cv::BORDER_CONSTANT, seam_masks_warped[i]);
		}

		prepare_exposure_compensator(seam_corners, seam_warped, seam_masks_warped);
		find_seams(seam_corner, seams_warped, seam_masks_warped);

		mSeamMasks.resize(num_images);
		for (int i = 0; i < num_images; i++) {
			cv::dilate(seam_masks_warped[i], mSeamMasks[i], cv::Mat());
		}
	}	

	prepare_composition(images, camera_params);

	return true;
 }

 void PanoramaMaker::prepare_exposure_compensator(const std::vector<cv::Point>& corners,
												  std::vector<cv::UMat>& images_warped,
												  const std::vector<cv::UMat>& masks_warped) {
	 // Step 1: Create compensator
	 mExposureCompensator = cv::detail::ExposureCompensator::createDefault(mExposureCompensatorType);

	 // Step 2: Setup compensator parameters
	 mExposureCompensator->feed(corners, images, masks);

	 // Step 3: Compensate the exposure of the analyzing images, so we can find seam better.
	 for (size_t i = 0; i < images.size(); i++)
		 mExposureCompensator->apply(int(i), corners[i], imagse[i], masks[i]);
 }

 void PanoramaMaker::find_seams(const std::vector<cv::Point>& corners,
								const std::vector<cv::UMat>& images,
								std::vector<cv::UMat>& masks) {
	 // Step 1: Convert images to float
	 const size_t num_images = imagse.size();
	 std::vector<cv::UMat> images_float(num_images);
	 for (int i = 0; i < num_images; i++) {
		 images[i].convertTo(images_float[i], CV_32F);
	 }

	 // Step 2: Modify masks with the seam
	 cv::detail::GraphGutSeamFinder sf(cv::detail::GraphGutSeamFinderBase::COST_COLOR);

	 // Step 3: Update masks with the seam
	 sf.find(images_float, corners, masks);
 }

 void PanoramaMaker::prepare_composition(const std::vector<cv::UMat>& images,
							             std::vector<cv::detail::CameraParams>& camera_params) {

 }

