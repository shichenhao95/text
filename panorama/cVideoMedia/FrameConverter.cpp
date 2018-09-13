#include "cvideomedia/FrameConverter.hpp"

FrameConverter::FrameConverter( int srcW, int srcH, enum AVPixelFormat srcFormat,
								int dstW, int dstH, enum AVPixelFormat dstFormat,
								int flags)
{	
	// struct SwsContext* sws_getContext( int srcW,
	//									  int srcH,
	//									  enum AVPixelFormat srcFormat,
	//									  int dstW,
	//									  int dstH,
	//	                                  enum AVPixelFormat dstFormat,
	//	                                  int flags,
	//	                                  SwsFilter *srcFilter,
	//	                                  SwsFilter *dstFilter,
	//	                                  const double *param)
	// Function: Allocate and return an SwsContext.You need it to perform scaling/conversion operations using sws_scale().
	// Parameters:
	//		srcW -- the width of the source image
	//	    srcH -- the height of the source image
	//	    srcFormat -- the source image format
	// 	    dstW -- the width of the destination image
	//	    dstH -- the height of the destination image
	//	    dstFormat -- the destination image format
	//	    flags -- specify which algorithm and options to use for rescaling
	//	    param -- extra parameters to tune the used scaler For SWS_BICUBIC param[0] and [1] tune the shape of the basis function, param[0] tunes f(1) and param[1] f´(1) 
	//               For SWS_GAUSS param[0] tunes the exponent and thus cutoff frequency 
	//               For SWS_LANCZOS param[0] tunes the width of the window function

	mSwsContext = sws_getContext( srcW, srcH, srcFormat,
								  dstW, dstH, dstFormat,
		                          flags, 0, 0, 0);
}

FrameConverter::~FrameConverter() {
	if (mSwdContext) {
		// void sws_freeContext (struct SwsContext* swsContext)
		// Function: Free the swscaler context swsContext. If swsContext is NULL, then does nothing.
		sws_freeContext(mSwsContext);
		mSwsContext = 0;
	}
}