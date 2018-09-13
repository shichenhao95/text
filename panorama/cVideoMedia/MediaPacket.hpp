#ifndef MEDIAPACKET_HPP
#define MEDIAPACKET_HPP

class MediaPacket final {
public:
	MediaPacket() : mPacket({0}) {}
	~MediaPacket() {}

	AVPacket * avPacket() { return &mPacket; }

	// void av_packet_unref(AVPacket *pkt)
	// Wipe the packet. Unreference the buffer referenced by the packet and reset the remaining packet fields to their default values.
	void unref() { av_packet_unref(&mPacket); }

	int stream_index() const { return mPacket.stream_index;  }
private:
	AVPacket mPacket;
};

#endif /* MEDIAPACKET_HPP */