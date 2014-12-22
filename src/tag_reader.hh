#ifndef _TAG_READER_HH
#define _TAG_READER_HH

#include "tag.hh"

#include <Eina.hh>

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

namespace TagLib { namespace MPEG { class File; }}

namespace emc {

class tag_reader
{
public:
   tag_reader(std::function<void(const tag&)> tag_read);
   ~tag_reader();

   void tag_file(const std::string& file);

private:
   void process_files();
   void process_pending_files();
   void process_file(const std::string &path);
   void process_mp3(TagLib::MPEG::File *file, tag& new_tag);
   std::vector<char> get_mp3_artwork(TagLib::MPEG::File *file);

private:
   std::function<void(const tag&)> tag_read;
   volatile bool terminated;

   ::efl::eina::condition_variable pending_file;
   ::efl::eina::mutex pending_files_mutex;
   ::efl::eina::thread worker;
   std::queue<std::string> pending_files;
};

}

#endif
