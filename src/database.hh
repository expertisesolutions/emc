#ifndef _DATABASE_HH
#define _DATABASE_HH

#include <Esql_Model.hh>

#include <functional>
#include <string>
#include <unordered_map>
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

   esql::model_table& artists_get();
   esql::model_table& albums_get();
   esql::model_table& tracks_get();
   esql::model_table& settings_get();

private:
   void success();
   void failure();
   void notify(bool error);

   void on_load(bool error);
   void map_tables();
   void check_tables();
   void migrate(int version);
   void load_tables();
   void on_table_loaded(bool error);
   void get_version();
   void on_version_row_loaded(bool error, esql::model_row row);

   void migrate_from_version_0();
   void create_table(const schema::table &table_definition);
   void create_table_field(esql::model_table table, const schema::table &table_definition, const schema::field &field_definition);
   bool on_table_created(bool error, unsigned int actual_count, size_t expected_count, std::function<void()> migration_done);
   void set_version(int version);
   void on_version_table_loaded(bool error, esql::model_table version_table, int version);
   void load_version_row(bool error, esql::model_row row, int version);
   void on_version_table_row_loaded(bool error, esql::model_row row, int version);
   void on_version_table_row_setted(bool error, esql::model_row row, int version);

   void migrate_from_version_1();
   void create_version_table();
   bool on_version_table_created(bool error);


private:
   std::vector<std::function<void(bool)>> handlers;
   esql::model_database db;
   esql::model_table artists;
   esql::model_table albums;
   esql::model_table tracks;
   esql::model_table settings;

   std::unordered_map<std::string, esql::model_table> tables;
   size_t loading_tables_count;
};

}

#endif
