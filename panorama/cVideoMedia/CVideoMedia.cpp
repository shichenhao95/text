#include "cvideomedia/CVideoMedia.hpp"
#include <iostream>

extern "C" {
	#include "libavformat/avformat.h"
}

extern const char * CVIDEOMEDIA_VERSION_STR;
const char* const CVideoMedia::version_str = CVIDEOMEDIA_VERSION_STR;

CVideoMedia::CVideoMedia() {
	std::cout << "cvideomedia V " << CVIDEOMEDIA_VERSION_STR << std::endl;
	av_register_all();			// Register codec, mutex and so on
	avformat_network_init();	// Do global initializtion of network libraries.
}

CVideoMedia::~CVideoMedia() {
	avformat_network_deinit();	// Undo the initialization done by avformat_network_init.
}