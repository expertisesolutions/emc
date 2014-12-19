#ifndef _DATABASE_MAP_HH
#define _DATABASE_MAP_HH

#include "row_map.hh"

#include <functional>
#include <string>
#include <unordered_map>

namespace esql {
   class model_table;
   class model_row;
}

namespace emc {

class database;

/**
 * Associates all database tables rows and their key values
 */
class database_map
{
public:
   database_map(database &db);
   ~database_map();

   /**
    * Starts mapping the rows and keys asynchronously
    * @param handler The callback handler
    */
   void async_map(std::function<void()> handler);

   /**
    * Tells if the asynchronously map has finished
    * @return [description]
    */
   bool is_mapped() const;

   row_map& artists_map_get();
   const row_map& artists_map_get() const;
   row_map& albums_map_get();
   const row_map& albums_map_get() const;
   row_map& tracks_map_get();
   const row_map& tracks_map_get() const;

private:
   void populate_map(esql::model_table &table, const std::string &key_field, row_map &map);
   void on_read_row(bool error, esql::model_row row, const std::string &key_field, row_map &map);
   void notify_if_finished();

private:
   database &db;
   std::function<void()> handler;
   int loading_rows_count;

   bool mapped;
   row_map artists_map;
   row_map albums_map;
   row_map tracks_map;
};

}

#endif
