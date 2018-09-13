#include "cvideomedia/MediaFrame.hpp"

bool MediaFrame::allocBuffer(const int width, const int height, const AVPixelFormat format) {
	if (mFrame->width > 0)
		unref();

	mFrame->width = width;
	mFrame->height = height;
	mFrame->format = format;

	// int av_frame_get_buffer ( AVFrame *frame,
	//							 int align)
	// Function: Allocate new buffer(s) for audio or video data.The following fields must be set on frame before calling this function:
	//           1) format (pixel format for video, sample format for audio)
	//			 2) width and height for video
	//	         3) nb_samples and channel_layout for audio
	// Parameters: 
	//		frame -- frame in which to store the new buffers.
	//		align -- required buffer size alignment
	// Return: 0 on success, a negative AVERROR on error.
	if (av_frame_get_buffer(mFrame, 32)) {
		mFrame->width = mFrame->height = 0;
		mFrame->format = AV_PIX_FMT_NONE;
		return false;
	}

	return true;
}