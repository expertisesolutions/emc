#ifndef _TAG_HH
#define _TAG_HH

#include <string>
#include <vector>

namespace emc {

struct tag
{
   std::string file;
   std::string title;
   int track;
   std::string artist;
   std::string album;
   std::string genre;
   int year;
   std::vector<unsigned char> artwork;
   int length;
   int bitrate;
   int samplerate;
   int channels;
};

}

#endif
