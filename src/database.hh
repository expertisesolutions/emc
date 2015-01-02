#ifndef _DATABASE_HH
#define _DATABASE_HH

#include "database_checker.hh"

#include <Esql_Model.hh>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace emc {

namespace schema {
   class table;
   class field;
}

class database
{
public:
   static const int INVALID_ID;

   database();
   ~database();

   void async_load(std::function<void(bool)> handler);
   void load_callback_add(std::function<void(bool)> handler);
   static void async_create_row(esql::model_table &table,
                                std::function<void(bool, esql::model_row)> callback);

   esql::model_database& database_get() const;
   esql::model_table& artists_get() const;
   esql::model_table& albums_get() const;
   esql::model_table& tracks_get() const;
   esql::model_table& settings_get() const;
   esql::model_table& version_get() const;

   esql::model_table table_get(const std::string &table) const;
   bool is_empty() const;

   /**
    * Creates tables by definitions
    * @param handler The callback function to call once on success or error
    */
   void async_create_tables(const std::vector<const schema::table*> &table_definitions,
                            std::function<void(bool)> handler);

   /**
    * Reset artist, album and track tables
    * @param handler The callback function to call once on success or error
    */
   void async_reset_media_tables(std::function<void(bool)> handler);

private:
   void success();
   void failure();
   void notify(bool error);

   void on_database_loaded(bool error);
   void on_tables_loaded(bool error);
   void on_database_checked(bool error);
   void map_tables();
   void check_tables();
   void on_child_added(const Emodel_Children_Event &event);

   void create_table(const schema::table &table_definition);
   void create_table_field(esql::model_table table, const schema::table &table_definition, const schema::field &field_definition);

private:
   std::vector<std::function<void(bool)>> handlers;
   mutable esql::model_database db;
   mutable esql::model_table artists;
   mutable esql::model_table albums;
   mutable esql::model_table tracks;
   mutable esql::model_table settings;
   mutable esql::model_table version;
   std::unordered_map<std::string, esql::model_table> tables;

   database_checker checker;
   ::efl::eo::signal_connection child_added_connection;
};

}

#endif
