#include <iostream>
#include "cvideomedia/MediaFrame.hpp"
#include "cvideomedia/RealtimeMediaInput.hpp"

const auto DELAY_BETWEEN_RETRY = std::chrono::seconds(5);

RealtimeMediaInput::RealtimeMediaInput(const char *src, const MediaType media_type, MediaInput * source)
	: mSrc(src), mMediaType(media_type), mSource(source), mInterrupted(false)
{
	mThread = std::thread(&RealtimeMediaInput::threadFun, this);
}

bool RealtimeMediaInput::readFrame(MediaFrame & frame) {
	std::unique_lock<std::mutex> lock(mMutex);
	do {
		while (!mInterrupted && mLastFrame == mFrameCount) {
			mCondVar.wait(lock);
		}

		if (mInterrupted)
			return false;

		frame.swap(mSharedFrame);
		mLastFrame = mFrameCount;

	} while (frame.width() == 0);

	return true;
}

void RealtimeMediaInput::threadFun() 
{
	std::cout << "Start thread\n";

	MediaFrame frame;

	while (!mInterrupted)
	{
		if (!mSource)
		{
			std::cout << "	[thread] Opening Source: " << mSrc << endl;
			MediaInput * newSource = MediaFactory::openVideoInput(mSrc.c_str(), mMediaType);
			if (!newSource) {
				std::cout << "	[thread] Fail to open video!" << std::endl;
				std::this_thread::sleep_for(DELAT_BETWEEN_RETRY);
				continue;
			}

			std::cout << "	[thread] Source Opened: " << mSrc.c_str() << std::endl;
			mSource = newSource;
		}

		while (!mInterrupted && mSource->readFrame(frame)) {
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mSharedFrame.swap(frame);
				++mFrameCount;
			}
			
			mCondVar.notify_one();
			frame.unref();
		}

		if (mInterrupted) {
			std::cout << "	[thread] Interrupted, deleting and leaving " << mSrc << std::endl;
		}
		else {
			std::cout << "	[thread] No more frames, deleting and retrying " << mSrc << std::endl;
		}

		delete mSource;
		mSource = nullptr;
	}
	std::cout << "Thread finished!" << std::endl;
}