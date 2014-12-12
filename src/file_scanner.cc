#include "file_scanner.hh"

#include "logger.hh"

#include <fileref.h>
#include <tag.h>

#include <Ecore.hh>
#include <Ecore_File.h>

#include <cstdint>
#include <cstdlib>
#include <functional>

namespace emc {

file_scanner::file_scanner(std::function<void(tag)> media_file_add_cb)
   : media_file_add_cb(media_file_add_cb)
   , terminated(false)
   , path_worker(&file_scanner::process_paths, this)
   , file_worker(&file_scanner::process_files, this)
{}

file_scanner::~file_scanner()
{
   terminated = true;
   pending_path.notify_one();
   path_worker.join();
   pending_file.notify_one();
   file_worker.join();
}

void file_scanner::start()
{
   const auto paths = get_configured_paths();
   for (auto &path : paths)
     scan_path(path);
}

std::vector<std::string> file_scanner::get_configured_paths() const
{
   // TODO: The path must be configurable
   const char *home_dir = getenv("HOME");
   if (!home_dir)
     home_dir = getenv("USERPROFILE");

   DBG << "Home directory: " << home_dir;

   std::string music_dir = std::string(home_dir) + "/Music";

   return std::vector<std::string>(1, music_dir);
}

void file_scanner::scan_path(const std::string &path)
{
   {
      efl::eina::unique_lock<efl::eina::mutex> lock(pending_paths_mutex);
      pending_paths.push(path);
   }
   pending_path.notify_one();
}


void file_scanner::process_paths()
{
   efl::eina::unique_lock<efl::eina::mutex> lock(pending_paths_mutex);
   while (!terminated)
     {
        DBG << "Waiting for new paths";
        pending_path.wait(lock);
        if (terminated) return;

        process_pending_paths();
     }
}

void
file_scanner::process_pending_paths()
{
   while (!pending_paths.empty() && !terminated)
     {
        DBG << "Processing " << pending_paths.size() << " path(s)...";
        auto path = pending_paths.front();
        pending_paths.pop();
        process_path(path);
     }
}

void
file_scanner::process_path(const std::string &path)
{
   DBG << "Processing path: " << path;
   if (!ecore_file_is_dir(path.c_str()))
     {
        WRN << "Not valid path: " << path;
        return;
     }

   auto it = eina_file_stat_ls(path.c_str());
   if (!it)
     {
        WRN << "Not valid path: " << path;
        return;
     }

   Eina_File_Direct_Info *info;
   EINA_ITERATOR_FOREACH(it, info)
     {
        if (info->type == EINA_FILE_DIR)
          pending_paths.push(info->path);
        else
          check_media_file(info->path);
     }
   eina_iterator_free(it);
}

void file_scanner::check_media_file(const std::string &path)
{
   {
      efl::eina::unique_lock<efl::eina::mutex> lock(pending_files_mutex);
      pending_files.push(path);
   }
   pending_file.notify_one();
}

void file_scanner::process_files()
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
file_scanner::process_pending_files()
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
file_scanner::process_file(const std::string &path)
{
   DBG << "Checking if filename is a recognized media type: " << path;

   TagLib::FileRef file(path.c_str());
   if (file.isNull() || !file.tag())
     return;

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
