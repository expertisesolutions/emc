#ifndef _TAGGING_SERVICE_HH
#define _TAGGING_SERVICE_HH

#include "bounded_buffer.hh"
#include "database_map.hh"
#include "file_scanner.hh"
#include "tag.hh"
#include "tag_reader.hh"

#include <Eina.hh>

namespace emc {

class database;
class tag_consumer;

/**
 * Scans for media files and inserts/updates database media information
 */
class tagging_service
{
public:
   tagging_service(::emc::database &database);
   ~tagging_service();

   void start();

private:
   void on_database_mapped();

private:
   ::emc::database &database;
   ::emc::database_map database_map;
   ::emc::bounded_buffer<std::string> files;
   ::emc::bounded_buffer<tag> tags;

   ::emc::file_scanner scanner;
   ::emc::tag_reader reader;
   std::shared_ptr<::emc::tag_consumer> consumer;
};

}

#endif
