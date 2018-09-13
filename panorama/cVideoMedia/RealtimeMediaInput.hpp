#ifndef RAELTIMEMEDIAINPUT_HPP
#define RAELTIMEMEDIAINPUT_HPP

#include <thread>

class Mediainput;
class MediaType;

class RealtimeMediaOnput {
public:
	bool readFrame(MediaFrame & frame);
private:
	friend class MediaFactory;
	RealtimeMediaInput(const char *src, const MediaType media_type, MediaInput *source);

	void threadFun();

	const std::string mSrc;
	const MediaType mMediaType;

	MediaInput * mSource;
	MediaFrame mSharedFrame;

	int mFrameCount;	// This is the last frame that was decoded and pushed
	int mlastFrame;		// This is the last frame Id that was readed 

	volatile bool mInterrupted;

	std::thread mThread;
	std::mutex mMutex;
	std::condition_variable mCondVar;
};

#endif /* RAELTIMEMEDIAINPUT_HPP */