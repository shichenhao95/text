#ifndef MEDIAINPUT_HPP
#define MEDIAINPUT_HPP

class MediaInput {
public:
	bool readFrame(MediaFrame & frame);
	bool readPacket(MediaPacket & packet);
private:
	friend class MediaFactory;

	MediaInput(const char * src, AVFormatContext * format_context, AVCodecContext **decoders = nullptr);

	std::string mSrc;
	AVFormatContext * mFormatContext;
	AVCodecContext ** mDecoders;
	AVCodecContext * mCurrentDecoder;
	AVStream *mVideoStream;
	int64_t mStreamStartTimestamp;
};

#endif /* MEDIAINPUT_HPP */