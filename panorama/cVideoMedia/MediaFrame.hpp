#ifndef MEDIAFRAME_HPP
#define MEDIAFRAME_HPP

class MediaFrame {
public:
	MediaFrame(){
		// AVFrame * av_frame_alloc(void)
		// Function: Allocate an AVFrame and set its fields to default values.
		mFrame = av_frame_alloc();
	}

	virtual ~MediaFrame(){
		if (mFrame != 0)
			av_frame_free(&mFrame);
	}

	void swap(MediaFrame & other) {
		std::swap(mFrame, other.mFrame);
	}

	// int av_frame_make_writable(AVFrame * frame)
	// Function: Ensure that the frame data is writable, avoiding data copy if possible.Do nothing if the frame is writable, allocate new buffers and copy the data if it is not.
	// Return: 0 on success, a negative AVERROR on error.
	void makeWritable() {
		av_frame_make_writable(mFrame);
	}
	
	AVFrame * avframe() { return mFrame; }

	void unref() { av_frame_unref(mFrame);  }

	bool allocBuffer(const int width, const int height, const AVPixelFormat format);
private:
	AVFrame * mFrame;
};

#endif /* MEDIAFRAME_HPP */