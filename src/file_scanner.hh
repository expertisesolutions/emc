#ifndef _FILE_SCANNER_HH
#define _FILE_SCANNER_HH

#include <Eio.h>
#include <Emodel.h>
#include <Emodel.hh>
#include <eio_model.eo.hh>

#include <functional>
#include <memory>
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
   file_scanner(std::function<void(const tag&)> media_file_add_cb);
   ~file_scanner();

   void start();

private:
   std::vector<std::string> get_configured_paths() const;
   void scan_path(const std::string &path);
   bool file_found(eio::model &file_model, void *info);
   bool file_status(eio::model &file_model, void *info);
   void check_media_file(const std::string &path);

private:
   std::vector<std::unique_ptr<eio::model>> files;
   std::function<void(const tag&)> media_file_add_cb;
};

}

#endif
