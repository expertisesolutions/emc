#include "tag_reader.hh"

#include "logger.hh"

#include <fileref.h>
#include <tag.h>

#include <Ecore.hh>
#include <Ecore_File.h>

namespace emc {

tag_reader::tag_reader(std::function<void(const tag&)> media_file_add_cb)
   : media_file_add_cb(media_file_add_cb)
   , terminated(false)
   , worker(&tag_reader::process_files, this)
{}

tag_reader::~tag_reader()
{
   terminated = true;
   pending_file.notify_one();
   worker.join();
}

void
tag_reader::tag_file(const std::string& file)
{
   {
      efl::eina::unique_lock<efl::eina::mutex> lock(pending_files_mutex);
      pending_files.push(file);
   }
   pending_file.notify_one();
}

void
tag_reader::process_files()
{
   efl::eina::unique_lock<efl::eina::mutex> lock(pending_files_mutex);
   while (!terminated)
     {
        DBG << "Waiting for new files";
        pending_file.wait(lock);
        if (terminated) return;

        process_pending_files();
     }
}

void
tag_reader::process_pending_files()
{
   while (!pending_files.empty() && !terminated)
     {
        DBG << "Processing " << pending_files.size() << " file(s)...";
        auto path = pending_files.front();
        pending_files.pop();
        process_file(path);
     }
}

void
tag_reader::process_file(const std::string &path)
{
   DBG << "Checking if filename is a recognized media type: " << path;

   TagLib::FileRef file(path.c_str());
   if (file.isNull() || !file.tag())
     return;

   // TODO: ignore if its already read

   TagLib::Tag *tag = file.tag();

   auto to_string = [](const TagLib::String &str) -> std::string
     {
        if (str == TagLib::String::null)
          return "";

        const auto UNICODE = true;
        return str.to8Bit(UNICODE);
     };

   ::emc::tag new_tag;
   new_tag.file = path;
   new_tag.title = to_string(tag->title());
   new_tag.track = tag->track();
   new_tag.artist = to_string(tag->artist());
   new_tag.album = to_string(tag->album());
   new_tag.genre = to_string(tag->genre());
   new_tag.year = tag->year();

   efl::ecore::main_loop_thread_safe_call_async(std::bind(media_file_add_cb, new_tag));
}

}
