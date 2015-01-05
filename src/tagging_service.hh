#ifndef _TAGGING_SERVICE_HH
#define _TAGGING_SERVICE_HH

#include <memory>

namespace emc {

class database;
class database_map;
class tagging_context;

/**
 * Managers the tagging context to scan paths for media files
 */
class tagging_service
{
public:
   tagging_service(::emc::database &database,
                   ::emc::database_map &database_map);
   ~tagging_service();

   /**
    * Scans the provided path for media files
    * @param path The path to scan for
    */
   void scan(const std::string &path);

private:
   void create_context(const std::string &path);

private:
   ::emc::database &database;
   ::emc::database_map &database_map;
   std::unique_ptr<tagging_context> context;
};

}

#endif
