#ifndef MEDIAFACTORY_HPP
#define MEDIAFACTORY_HPP

class RealtimeMediaInput;

class MediaFactory {
public:

	static MediaInput * openVideoInput( consr char *src,
										const MediaType, media_type);

	static RealtimeMediaInput * openRealtimeVideoInput( const char * src,
														const MediaType media_type,
														const bool async = false);

};

#endif /* MEDIAFACTORY_HPP */