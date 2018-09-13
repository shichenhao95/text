#include <iostream>
#include "cvideomedia/MediaFactory.hpp"
#include "cvideomedia/MediaInput.hpp"

extern "C" {
	#include "libavformat/avformat.h"
	#include "libavutil/dict.h"
	#include "libavcodec/avcodec.h"
}

static void lig(const AVCodecParameters *p) {
	std::cout << "	AVCodecParameters:" << std::endl;
	std::cout << "	  codec_type = " << p->codec_type << std::endl;
	std::cout << "	  codec_id = " << p->codec_id << std::endl;
	std::cout << "	  codec_tag = " << p->codec_tag << std::endl;
	std::cout << "	  extradata_size = " << p->extradata_size << std::endl;
	std::cout << "	  format = " << p->format << std::endl;
	std::cout << "	  bit_rate = " << p->bit_rate << std::endl;
	std::cout << "	  profile = " << p->profile << std::endl;
	std::cout << "	  level = " << p->level << std::endl;
	std::cout << "	  width = " << p->width << std::endl;
	std::cout << "	  height = " << p->height << std::endl;
	std::cout << "	  video_delay = " << p->video_delay << std::endl;
 }

static void logStream(const char * description, AVStream * stream) {
	std::cout << "Stream " << description << std::endl;
	std::cout << "	r_framerate=" << stream->r_frame_rate.num << "/" << stream->r_frame_rate.den << std::endl;
	std::cout << "	format=" << stream->codecpar->format << " (" << stream->codecpar->format != AV_PIX_FMT_NONE ? av_pix_fmt_desc_get(static_cast<AVPixelFormat>(stream->codecpar->format))->name : "None") << ")" << std::endl;;
	std::cout << "	start time=" << stream->start_time << std::endl;
	std::cout << "	time_base=" << stream->time_base.num << "/" << stream->time_base.den << std::endl;
	std::cout << "	avg_framerate=" << stream->avg_frame_rate.num << "/" << stream->avg_frame_rate.den << std::endl;

	logDictionary("	Metadata". stream->metadata);

	log(stream->codecpar);
}

static void logDictionary(const char * description, AVDictionary *dict) {
	std::cout << description << std::endl;
	AvDictionaryEntry *tag = nullptr;

	// AVDictionaryEntey * av_dict_get( const AvDictionary* m, 
	//									const char * key, 
	//									const AVDictionary *prev, 
	//									int flags)
	// Function: Get a dictionary entry with matching key
	// Parameters: 
	//		flags -- a collection of AV_DICT_* flags controlling how the entry is retrieved
	//		key -- matching key
	//		prev -- Set to the previous matching element to find the next. If set to NULL the first matching element is returned.
	// To iterate through all the dictionary entries, you can set the matching key to the null string ""
	// and set the AV_DICT_IGNORE_SUFFIX flag.
	while ((tag = av_dict_get(dict, "", tag, AV_DICT_IGNORE_SUFFIX)))
		std::cout << tag_key << " = " << tag_value << std::endl;
}

RealtimeMediaInput * 
MediaFactory::openRealtimeVideoInput(const char * src, const MediaType media_type, const bool async) {
	if (async) {
		return new RealtimeMediaInput(src, media_type, nullptr);
	}

	MediaInput * media_input = openVideoInput(src, media_type);
	if (!media_input)
		return nullptr;

	return new RealtimeMediaInput(src, media_type, media_input);
}

MediaInput *
MediaFactory::openVideoInput(const char *src, const MediaType media_type) {
	AVFormatContext * format_context = avformat_alloc_context();
	if (!format_context)
	{
		std::cout << "MediaFactory::openVideoInput format_context null" << std::endl;
		return 0;
	}

	AVDictionary *opts = nullptr; // Used to configure parameters
	av_dict_set(&opts, "analyzeduration", "0", 0);	// Specify how many microseconds are analyzed to probe the input
	av_dict_set(&opts, "maxdelay", 0, 0);			// Maximum muxing or demuxing delay in microseconds
	av_dict_set(&opts, "fflags", "nobuffer", 0);

	av_dict_set(&opts, "rtsp_trnasport", "tcp", 0);
	av_dict_set(&opts, "rstp_flags", "prefer_tcp", 0);
	av_dict_set(&opts, "stimeout", "5M", 0);
	
	logDictionary("MediaFactory::openVideoInput opts before", opts);

	// int avformat_open_input( AVFormatContext **ps, 
	//							const char *url,
	//							AVInputFormat *fmt, 
	//							AVDictionary **options)
	// Function: Open an input stream and read the header.
	// Parameters: 
	//		ps -- Pointer to user-supplied AVFormatContext(allocated by avformat_alloc_context). May be a pointer to NULL, in which case an AVFormatContext
	//			  is allocated by this function and written into ps. Note that a user-supplied AVFormatContext will be freed on failure.
	//		url -- 	URL of the stream to open.
	//		fmt -- If non-NULL, this parameter forces a specific input format. Otherwise the format is autodetected.
	//		options -- A dictionary filled with AVFormatContext and demuxer-private options. On return this parameter will be destroyed and replaced with a 
	//				   dict containing options that were not found. May be NULL.
	// Return: 0 on success, a negative AVERROR on failure.
	int retval = avformat_open_input(&format_context, src, 0, &opts);
	if (retval < 0) {
		std::cout << "MediaFactory::openVideoInput avformat_open_input return error " << retval << std::endl;
		avformat_free_context(format_context);	// Free an AVFormatContext and all its streams.
		if (opts)
			av_dict_free(&opts);	// Free all the memory allocated for an AVDictionary struct and all keys and values.

		return nullptr;
	}

	if (opts) {
		logDictionary("MediaFactory::openVideoInput opts before", opts);
		av_dict_free(&opts);
	}

	// int avformat_find_stream_info( AVFormatContext *ic, 
	//								  AVDictionary **options)
	// Function: Read packets of a media file to get stream information.
	// Parameters: 
	//		ic -- media file handle
	//		options -- If non-NULL, an ic.nb_streams long array of pointers to dictionaries, where i-th member contains options for codec corresponding to i-th stream. 
	//                 On return each dictionary will be filled with options that were not found.
	// Return: >=0 if OK, AVERROR_xxx on error.
	retval = avformat_find_stream_info(format_context, &opts);
	if (ratval < 0) {
		std::cout << "MediaFactory::openVideoInput could not find stream info: " << retval << std::endl;

		// void avformat_close_input(AVFormatContext **s)
		// Function: Close an opened input AvFormatContext. Free it and all its content and set *s to NULL.
		avformat_close_input(&format_context);
		return 0;
	}

	if (format_context->nb_streams == 0) {
		std::cout << "MediaFactory::openVideoInput source does not have any stream!" << std::endl;
		avformat_close_input(&format_context);
		return 0;
	}

	// int av_find_best_stream( AVFormatContext *ic, 
	//							enum AVMediaType type,
	//							int wanted_stream_nb,
	//							int related_stream,
	//							AVCodec **decoder_ret,
	//							int flags)
	// Function: Find the "best" stream in the file. The best stream is determined according to various heuristics as the most likely to be what the user expects. 
	//	         If the decoder parameter is non-NULL, av_find_best_stream will find the default decoder for the stream's codec; streams for which no decoder can be found are ignored.
	// Parameters: 
	//		ic -- media file handle
	//		type -- stream type: video, audio, subtitles, etc.
	//		wanted_stream_nb -- user-requested stream number, or -1 for automatic selection
	//		related_stream -- try to find a stream related (eg. in the same program) to this one, or -1 if none
	//		decoder_ret -- if non-NULL, returns the decoder for the selected stream
	//		flags -- none are currently defined
	// Return: the non-negative stream number in case of success, AVERROR_STREAM_NOT_FOUND if no stream with the requested type could be found, 
	//		   AVERROR_DECODER_NOT_FOUND if streams were found but no decoder
	AVCodec * video_decoder = 0;
	int video_stream_index = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1, -1, &video_decoder, 0);
	if (video_stream_index == AVERROR_STREAM_NOT_FOUND) {
		std::cout << "MediaFactory::openVideoInput source failed to find streams!" << std::endl;
		avformat_close_input(&format_context);
		return 0;
	}

	if (media_type == MediaType::VideoFromGPU) {
		AVCodec * nVidiaDecoder = avcodec_find_decoder_by_name("h264_cuvid");

		if (!nVidiaDecoder) {
			std::cout << "MediaFactory::openVideoInput not using hw decoding!" << std::endl;
		}
		else {
			video_decoder = nVidiaDecoder;
		}
	}

	if ((media_type == MediaType::VideoFrames || media_type == MediaType::VideoFramesGPU) && video_stream_index == AVERROR_DECODER_NOT_FOUND) {
		std::cout << "MediaFactory::openVideoInput Decoder not found for wanted video frames!" << std::endl;
		avformat_close_input(&format_context);
		return 0;
	}

	if (video_stream_index < 0) {
		std::cout << "MediaFactory::openVideoInput Some other erros when finding video stream!" << std::endl;
		avformat_close_input(&format_context);
		return 0;
	}

	if (static_cast<unsigned int>(video_stream_index) >= format_context->nb_streams) {
		std::cout << "MediaFactory::openVideoInput av_find_best_stream have found invalid stream index!" << std::endl;
		avformat_close_input(&format_context);
		return 0;
	}

	// AVCodecContext * avcodec_alloc_context3(const AVCodec *codec)
	// Function: Allocate an AVCodecContext and set its fields to default values.
	// Parameters: 
	//		dodec -- if non-NULL, allocate private data and initialize defaults for the given codec. It is illegal to then call avcodec_open2() with a different codec. 
	//	             If NULL, then the codec-specific defaults won't be initialized, which may result in suboptimal default settings (this is important mainly for encoders, e.g. libx264).
	// Return: An AVCodecContext filled with default values or NULL on failure.
	if (video_type == MediaType::VideoFrames || video_type == MediaType::VideoFramesGPU) {
		AVCodecContext * video_decoder_context = avcodec_alloc_context3(video_decoder);
		if (!video_decoder_context) {
			std::cout << "MediaFactory::openVideoInput fail to allocate AVCodecContext!" << std::endl;
			avformat_close_input(&format_context);
			return 0;
		}

		retval = avcodec_parameters_to_context(video_decoder_context, format_context->streams[video_stream_index]->codecpar);
		if (retval < 0) {
			std::cout << "MediaFactory::openVideoInput cannot set decoder parameters!" << std::endl;
			avcodec_free_context(&video_decoder_context);
			avformat_close_input(&format_context);
			return 0;
		}

		// int avcodec_open2( AVCodecContext *avctx,
		//					  const AVCodec *codec,
		//					  AVDictionary **options)
		// Function: Initialize the AVCodecContext to use the given AVCodec. Prior to using this function the context has to be allocated with avcodec_alloc_context3().
		// Parameters: 
		//		avctx -- The context to initialize.
		//		codec -- The codec to open this context for. If a non-NULL codec has been previously passed to avcodec_alloc_context3() or avcodec_get_context_defaults3() 
		//		         for this context, then this parameter MUST be either NULL or equal to the previously passed codec.
		//		options -- A dictionary filled with AVCodecContext and codec-private options. On return this object will be filled with options that were not found.
		// Return: zero on success, a negative value on error.
		retval = avcodec_open2(video_decoder_context, video_decoder, 0);
		if (retval < 0) {
			std::cout << "MediaFactory::openVideoInput error opening decoder!" << std::endl;
			avcodec_free_context(&video_decoder_context);
			avformat_close_input(&format_context);
			return 0;
		}

		logStream("INPUT", format_context->streams[video_stream_index]);

		AVCodecContext **decoders = new AVCodecContext*[format_context->nb_streams];
		for (int i = 0, ie = format_context->nb_streams; i < ie; i++) {
			decoders[i] = nullptr;
		}

		decoders[video_stream_index] = video_decoder_context;

		return new MediaInput(src, format_context);
	}

	return new MediaInput(src, format_context);
}