#include<iostream>
#include<chrono>
#include "cvideomedia/MediaInput.hpp"


MediaInput::MediaInput(const char * src, AVFormatContext * format_context, AVCodecContext **decoders)
	: mSrc(src), mFormatContext(format_context), mDecoders(decoders), mCurrentDecoder(0), mVideoStream(0)
{
	mStreamStartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	int video_stream_index = av_find_best_stream(mFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, 0, 0);
	if (static_cast<unsigned int> (video_stream_index) < mFormatContext->nb_streams) {
		mVideoStream = mFormatContext->streams[video_stream_index];
	}
}

bool MediaInput::readPacket(MediaPacket & packet) {
	// int av_read_frame ( AVFormatContext *s,
	//					   AVPacket *pkt)
	// Function: Return the next frame of a stream. This function returns what is stored in the file, and does not validate that what is there are valid frames for the decoder.
	//           It will split what is stored in the file into frames and return one for each call. It will not omit invalid data between valid frames so as to give the decoder the maximum information possible for decoding.
	//           If pkt->buf is NULL, then the packet is valid until the next av_read_frame() or until avformat_close_input().Otherwise the packet is valid indefinitely.In both cases the packet must be freed with av_packet_unref 
	//			 when it is no longer needed.For video, the packet contains exactly one frame.For audio, it contains an integer number of frames if each frame has a known fixed size(e.g.PCM or ADPCM data).If the audio frames have 
	//           a variable size(e.g.MPEG audio), then it contains one frame. pkt->pts, pkt->dts and pkt->duration are always set to correct values in AVStream.time_base units(and guessed if the format cannot provide them).pkt->pts 
	//           can be AV_NOPTS_VALUE if the video format has B - frames, so it is better to rely on pkt->dts if you do not decompress the payload.
	return av_read_frame(mFormatContext, packet.avpacket()) == 0;
}

bool MediaInput::readFrame(MediaFrame & frame) {
	if (!mDecoders)
		return false;

	int result;
	AVFrame * f = frame.avframe();

	do
	{
		if (mCurrentDecoder) {
			result = avcodec_receive_frame(mCurrentDecoder, f);
			switch (result) {
				case 0:
					// Got a frame!
					return true;
				
				case AVERROR(EAGAIN):
					// This decoder does not have more frames, go to the read frame loop.
					mCurrentDecoder = 0;
					break;
				
				case AVERROR_EOF:
					std::cout << "readFrame:avcodec_receive_frame EOF " << AVERROR_EOF << std::endl;
					return false;

				case AVERROR(EINVAL):
					std::cout << "readFrame:avcodec_receive_frame EINVAL" << AVERROR(EINVAL) << std::endl;
					return false;

				default:
					char str[1024] = {0};
					// static char* av_make_error_string ( char *errbuf,
					//									   size_t errbuf_size,
					//									   int errnum)
					// Function: Fill the provided buffer with a string containing an error string corresponding to the AVERROR code errnum.
					// Parameter: errbuf -- a buffer
					//			  errbuf_size -- size in bytes of errbuf
					//			  errnum -- error code to describe
					// Return: the buffer in input, filled with the error description
					std::cout << "readFrame:avcodec_receive_frame (" << result << ")= " << av_make_error_string(str, 1024, result) << std::endl;
					return false;
			}
		}

		MediaPacket packet;
		// This will loop until we get a packet which has a decoder configured or some error happened
		while (!mCurrentDecoder && readPacket(packet)) {
			if (((unsigned int)(packet.stream_index()) < mFormatContext->nb_streams) && mDecoders[packets.stream_index()]) {
				mCurrentDecoder = mDecoders[packet.stream_index()];

				result = avcodec_send_packet(mCurrentDecoder, packet.avPacket());
				switch (result) {
					case 0:
						break;

					case AVERROR(EAGAIN):	// This should not happen
						std::cout << "readFrame:avcodec_send_packet error: Internal state error!" << std::endl;
						return false;

					case AVERROR_EOF:
						std::cout << "readFrame:avcodec_send_packet error: EOF " << AVERROR_EOF << std::endl;
						return false;

					case AVERROR(EINVAL):
						std::cout << "readFrame:avcodec_send_packet error: EINVAL" << AVERROR(EINVAL) << std::endl;
						return false;

					default:
						char str[1024] = { 0 };
						std::cout << "readFrame:avcodec_send_packet (" << result << ")= " << av_make_error_string(str, 1024, result) << std::endl;
						return false;
				}
			}
			packet.unref();
		}
	} while (mCurrentDecoder); // This will loop if readPacket got a packet which has a decoder.

	// We arrive here when there is no current decoder and readPacket returned false
	return false;
}