#ifndef _FILE_SCANNER_HH
#define _FILE_SCANNER_HH

#include <Eina.hh>

#include <functional>
#include <queue>
#include <string>
#include <vector>

namespace emc {

class file_scanner
{
public:
   file_scanner(std::function<void(const std::string&)> file_found);
   ~file_scanner();

   void start();

private:
   std::vector<std::string> get_configured_paths() const;
   void scan_path(const std::string &path);

   void process_paths();
   void process_pending_paths();
   void process_path(const std::string &path);

private:
   std::function<void(const std::string&)> file_found;
   volatile bool terminated;

   ::efl::eina::condition_variable pending_path;
   ::efl::eina::mutex pending_paths_mutex;
   ::efl::eina::thread worker;
   std::queue<std::string> pending_paths;
};

}

#endif
