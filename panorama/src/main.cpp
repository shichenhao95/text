#include "PanoramaMaker.hpp"
#include "cvideoMedia/CVideoMedia.hpp"
#include "cvideoMedia/MediaFactory.hpp
#include "cvideoMedia/RealtimeMediaInput.hpp"

#define IN_SERVER_BASE "rtmp://192.168.2.171:1554/live/"
#define OUT_SERVER_BASE "rtmp://192.168.2.171:1554/live/"

void avframeToMat(MediaFrame &src_frame, MediaFrame &rgb_frame, cv::Mat &image);

int main(int argc, char **argv) {

	CVideoMedia cVideoMedia;

	std::vector<RealtimeMediaInput *> media_inputs;
	std::vector<const char *> media_outputs;
	std::vector<MediaFrame> input_frames(media_inputs.size());
	std::vector<MediaFrame> rgb_frames(media_inputs.size());
	std::vector<cv::Mat> input_images(media_inputs.size());
	std::vector<cv::UMat> undistored_images;

	media_inputs.push_back(MediaFactory::openRealtimeVideoInput(IN_SERVER_BASE "cam8_0r", MediaType::VideoFrames, true));
	media_inputs.push_back(MediaFactory::openRealtimeVideoInput(IN_SERVER_BASE "cam8_1r", MediaType::VideoFrames, true));
	media_inputs.push_back(MediaFactory::openRealtimeVideoInput(IN_SERVER_BASE "cam8_2r", MediaType::VideoFrames, true));
	media_inputs.push_back(MediaFactory::openRealtimeVideoInput(IN_SERVER_BASE "cam8_3r", MediaType::VideoFrames, true));
	media_inputs.push_back(MediaFactory::openRealtimeVideoInput(IN_SERVER_BASE "cam8_4r", MediaType::VideoFrames, true));
	media_inputs.push_back(MediaFactory::openRealtimeVideoInput(IN_SERVER_BASE "cam8_5r", MediaType::VideoFrames, true));
	media_outputs.push_back(OUT_SERVER_BASE "cam8_panorama");

	for (int i = 0; i < media_inputs.size(); i++) {
		if (!media_inputs[i]) {
			std::cout << "Couldn't open input " << media_inputs[i] << std::endl;
			return 1;
		}

		while (!media_inputs[i]->readFrame(input_frames[i]) || input_frames[i].width == 0)
			;
		input_frames[i].makeWritable();
		avframeToMat(input_frames[i], rgb_frames[i], input_images[i]);
		undistorted_images.push_back(input_images[i].getUMat(cv::ACCESS_READ));
	}

	PanoramaMaker pmaker;

	if (!pmaker.analyze(undistorted_images, -1, 0, 01)) {
		std::cout << "Error analyzing" << std::endl;
		return -1;
	} else {

	}

	return 0;
}

void avframeToMat(MediaFrame &src_frame, MediaFrame &rgb_frame, cv::Mat &image) {
	const AVFrame *frame = src_frame.avframe();
	rgb_frame.allocBuffer(frame->width, frame->height, AV_PIX_FMT_BGR24);
	FrameConverter frameConverter(frame->width, frame->height, static_cast<AVPixelFormat> (frame->format),
		frame->width, frame->height, AX_PIX_FMT_BGR24);
	frameConverter.convert(src_frame, rgb_frame);

	/** @overload
	@param rows Number of rows in a 2D array.
	@param cols Number of columns in a 2D array.
	@param type Array type. Use CV_8UC1, ..., CV_64FC4 to create 1-4 channel matrices, or
	CV_8UC(n), ..., CV_64FC(n) to create multi-channel (up to CV_CN_MAX channels) matrices.
	@param data Pointer to the user data. Matrix constructors that take data and step parameters do not
	allocate matrix data. Instead, they just initialize the matrix header that points to the specified
	data, which means that no data is copied. This operation is very efficient and can be used to
	process external data using OpenCV functions. The external data is not automatically deallocated, so
	you should take care of it.
	@param step Number of bytes each matrix row occupies. The value should include the padding bytes at
	the end of each row, if any. If the parameter is missing (set to AUTO_STEP ), no padding is assumed
	and the actual step is calculated as cols*elemSize(). See Mat::elemSize.
	*/
	image = cv::Mat(rgb_frame.avframe()->height, rgb_frame.avframe()->width, CV_8UC3, (void*)rgb_frame.avframe()->data[0], rgb_frame.avframe()->linesize[0]);
}