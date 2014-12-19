#ifndef _ROW_MAP_HH
#define _ROW_MAP_HH

#include "Esql_Model.hh"

#include <string>
#include <unordered_map>

namespace emc {

/**
 * Maps the key values to its esql::model_row
 */
class row_map
{
public:
   row_map();
   ~row_map();

   /**
    * Associates the key value and model_row
    * @param key The key value
    * @param row The model_row
    */
   void add(const std::string &key, esql::model_row &row);

   /**
    * Finds the model_row associated to the key
    * @param key The key to search for
    * @return Returns the associated row if it exists or a empty row
    */
   esql::model_row find(const std::string &key) const;

   /**
    * Tells if the associated row exists
    * @param key The key to search for
    * @return Returns if the associated row exists
    */
   bool exists(const std::string &key) const;

private:
   std::unordered_map<std::string, esql::model_row> map;
};

}

#endif
