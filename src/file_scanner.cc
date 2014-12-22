#include "file_scanner.hh"

#include "logger.hh"

#include <Ecore.hh>
#include <Ecore_File.h>

#include <functional>

namespace emc {

file_scanner::file_scanner(bounded_buffer<std::string> &files)
   : files(files)
   , terminated(false)
   , worker(&file_scanner::process, this)
{}

file_scanner::~file_scanner()
{
   terminated = true;
   pending_path.notify_one();
   worker.join();
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
      pending_paths.push_back(path);
   }
   pending_path.notify_one();
}


void file_scanner::process()
{
   while (!terminated)
     {
        std::vector<std::string> paths;

        DBG << "Waiting for new paths";
        {
           efl::eina::unique_lock<efl::eina::mutex> lock(pending_paths_mutex);
           if (pending_paths.empty())
             pending_path.wait(lock);
           if (terminated) return;
           paths = move(pending_paths);
        }

        process_paths(paths);
     }
}

void
file_scanner::process_paths(const std::vector<std::string> &paths)
{
   DBG << "Processing " << paths.size() << " path(s)...";
   for (auto &path : paths)
     {
        if (terminated) return;
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
          scan_path(info->path);
        else
          files.push_back(info->path);
     }
   eina_iterator_free(it);
}

}
