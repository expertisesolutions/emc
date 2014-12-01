#ifndef _FILE_SCANNER_HH
#define _FILE_SCANNER_HH

#include <Eio.h>
#include <Emodel.h>
#include <Emodel.hh>
#include <eio_model.eo.hh>

#include <memory>
#include <string>
#include <vector>

namespace emc {

class settingsmodel;

class file_scanner
{
public:
   file_scanner();
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
};

}

#endif
