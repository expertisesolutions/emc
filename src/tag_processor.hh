#ifndef _TAG_PROCESSOR_HH
#define _TAG_PROCESSOR_HH

#include "row_map.hh"

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
   tag_processor(row_map &map,
                 esql::model_table &table,
                 const std::string &key_value,
                 std::function<void()> process_next,
                 std::function<void(esql::model_row)> property_set);
   ~tag_processor();

   /**
    * Starts the processing of key_value
    * @return Returns if the processing is pending
    */
   bool process();

private:
   void row_create_handler(bool error, esql::model_row row);
   void property_set_handler(bool error, esql::model_row row);

private:
   row_map &map;
   esql::model_table &table;
   const std::string key_value;
   std::function<void()> process_next;
   std::function<void(esql::model_row)> property_set;
};

}

#endif
