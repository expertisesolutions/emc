#ifndef _TAG_READER_HH
#define _TAG_READER_HH

#include "bounded_buffer.hh"
#include "picture_resizer.hh"
#include "tag.hh"

#include <Eina.hh>

#include <string>
#include <vector>

namespace TagLib { namespace MPEG { class File; }}

namespace emc {

/**
 * Consumes files and produces o tags
 */
class tag_reader
{
public:
   tag_reader(bounded_buffer<std::string> &files, bounded_buffer<tag> &tags);
   ~tag_reader();

private:
   void process();
   void process_file(const std::string &path);
   void process_mp3(TagLib::MPEG::File *file, tag& new_tag);
   std::vector<unsigned char> get_mp3_artwork(TagLib::MPEG::File *file);

private:
   picture_resizer resizer;
   bounded_buffer<std::string> &files;
   bounded_buffer<tag> &tags;
   ::efl::eina::thread worker;
};

}

#endif
