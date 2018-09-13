#ifndef FRAMECONVERTER_HPP
#define FRAMECONVERTER_HPP

#include "MediaFrame.hpp"

extern "C" {
	#include "libswscale/swscale.h" 
}

class FrameConverter {
public:
	FrameConverter( int srcW, int srcH, enum AVPixelFormat srcFormat,
					int dstW, int dstH, enum AVPixelFormat dstFormat,
					int flags = SWS_FAST_BILINER);

	~FrameConverter();

	void convert(MediaFrame &src, MediaFrame &dst) {
		// int sws_scale ( struct SwsContext *c,
		//                 const uint8_t *const srcSlice[],
		//	               const int srcStride[],
		//	               int srcSliceY,
		//				   int srcSliceH,
		//			       uint8_t *const dst[],
		//				   const int dstStride[])
		// Function: Scale the image slice in srcSlice and put the resulting scaled slice in the image in dst.
		//	         A slice is a sequence of consecutive rows in an image. Slices have to be provided in sequential order, either in top - bottom or bottom - top order.If slices are provided in non - sequential order the behavior of the function is undefined.
		// Parameters: 		
		//		c -- the scaling context previously created with sws_getContext()
		//		srcSlice -- the array containing the pointers to the planes of the source slice
		//		srcStride -- the array containing the strides for each plane of the source image
		//	    srcSliceY -- the position in the source image of the slice to process, that is the number(counted starting from zero) in the image of the first row of the slice
		//	    srcSliceH -- the height of the source slice, that is the number of rows in the slice
		//		dst	-- the array containing the pointers to the planes of the destination image
		//		dstStride -- the array containing the strides for each plane of the destination image
		// Return: the height of the output slice.
		sws_scale(mSwsContext,
			src.avframe()->data, src.avframe()->linesize, 0, src.height(),
			dst.avframe()->data, dst.avframe()->linesize);
	}
private:
	struct SwsContext *mSwsContext;
};

#endif /* FRAMECONVERTER_HPP */