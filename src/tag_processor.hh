#ifndef _TAG_PROCESSOR_HH
#define _TAG_PROCESSOR_HH

extern "C"
{
#include <Esskyuehl.h>
#include <Esql_Model.h>
}
#include <Esql_Model.hh>

#include <functional>
#include <string>
#include <unordered_map>

namespace emc {

/**
 * Inserts in a specified table the key_value if it doesn't exist
 */
class tag_processor
{
public:
   tag_processor(std::unordered_map<std::string, esql::model_row> &map,
                 esql::model_table &table,
                 const std::string &key_value,
                 std::function<void()> process_next,
                 std::function<void(esql::model_row)> property_set);
   ~tag_processor();

   /**
    * Starts the processing of key_value
    */
   void process();

private:
   void row_create_handler(bool error, esql::model_row row);
   void property_set_handler(bool error, esql::model_row row);

private:
   std::unordered_map<std::string, esql::model_row> &map;
   esql::model_table &table;
   const std::string key_value;
   std::function<void()> process_next;
   std::function<void(esql::model_row)> property_set;
};

}

#endif
