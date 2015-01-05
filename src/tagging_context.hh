#ifndef _TAGGING_CONTEXT_HH
#define _TAGGING_CONTEXT_HH

#include "bounded_buffer.hh"
#include "database_map.hh"
#include "file_scanner.hh"
#include "tag.hh"
#include "tag_reader.hh"

#include <string>

namespace emc {

class tag_consumer;

/**
 * This class wires all necessary classes to scan a path and updates the database
 */
class tagging_context
{
public:
   /**
    * Constructor
    * @param database The database to update tag information to
    * @param database_map The database map
    * @param max_files Max queued files
    * @param max_tags Max queued tags
    * @param path Tha path to scan
    */
   tagging_context(::emc::database &database,
                   ::emc::database_map &database_map,
                   size_t max_files,
                   size_t max_tags,
                   const std::string &path);
   ~tagging_context();

   /**
    * Starts scanning the provided path
    */
   void start();

   /**
    * Stops the scanner and closes the queue of scanning files
    */
   void stop();

private:
   const std::string path;
   ::emc::bounded_buffer<std::string> files;
   ::emc::bounded_buffer<tag> tags;

   ::emc::file_scanner scanner;
   ::emc::tag_reader reader;
   std::shared_ptr<tag_consumer> consumer;
};

}

#endif
