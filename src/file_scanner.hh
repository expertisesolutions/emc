#ifndef _FILE_SCANNER_HH
#define _FILE_SCANNER_HH

#include "bounded_buffer.hh"

#include <Eina.hh>

#include <functional>
#include <string>
#include <vector>

namespace emc {

class file_scanner
{
public:
   file_scanner(bounded_buffer<std::string> &files);
   ~file_scanner();

   void start();

private:
   std::vector<std::string> get_configured_paths() const;
   void scan_path(const std::string &path);

   void process();
   void process_paths(const std::vector<std::string> &paths);
   void process_path(const std::string &path);

private:
   bounded_buffer<std::string> &files;
   volatile bool terminated;

   ::efl::eina::condition_variable pending_path;
   ::efl::eina::mutex pending_paths_mutex;
   ::efl::eina::thread worker;
   std::vector<std::string> pending_paths;
};

}

#endif
