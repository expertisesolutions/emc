#ifndef _FILE_SCANNER_HH
#define _FILE_SCANNER_HH

#include <Eina.hh>
#include <Eio.h>
#include <Emodel.h>
#include <Emodel.hh>
#include <eio_model.eo.hh>

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

namespace emc {

class settingsmodel;

struct tag
{
   std::string file;
   std::string title;
   int track;
   std::string artist;
   std::string album;
   std::string genre;
   int year;
};

class file_scanner
{
public:
   file_scanner(std::function<void(tag)> media_file_add_cb);
   ~file_scanner();

   void start();

private:
   std::vector<std::string> get_configured_paths() const;
   void scan_path(const std::string &path);
   bool file_found(eio::model &file_model, void *info);
   bool file_status(eio::model &file_model, void *info);
   void check_media_file(const std::string &path);

   void process();
   void process_pending_files();
   void process_file(const std::string &path);

private:
   std::vector<std::unique_ptr<eio::model>> files;
   std::function<void(const tag&)> media_file_add_cb;
   ::efl::eina::condition_variable pending_file;
   ::efl::eina::mutex mutex;
   ::efl::eina::thread worker;
   bool terminated;
   std::queue<std::string> pending_files;
};

}

#endif
