#ifndef _DATABASE_CHECKER_HH
#define _DATABASE_CHECKER_HH

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

class database;

/**
 * Checks, validates and migrates the database schema
 */
class database_checker
{
public:
   database_checker(::emc::database &database);
   ~database_checker();

   void async_check(std::function<void(bool)> handler);

private:
   void success();
   void failure();
   void notify(bool error);

   void migrate(int version);
   void get_version();
   void on_version_row_loaded(bool error, esql::model_row row);

   void migrate_from_version_0();
   void create_table_field(esql::model_table table, const schema::table &table_definition, const schema::field &field_definition);
   void set_version(int version);
   void on_version_table_loaded(bool error, esql::model_table version_table, int version);
   void load_version_row(bool error, esql::model_row row, int version);
   void on_version_table_row_loaded(bool error, esql::model_row row, int version);
   void on_version_table_row_setted(bool error, esql::model_row row, int version);

   void migrate_from_version_1();
   void create_v2_tables();
   void on_v2_tables_created(bool error);

   void migrate_from_version_2();
   void on_migration_completed(bool error, int version);

   void async_create_table_fields(esql::model_table &table,
                                  const schema::table &table_definition,
                                  const std::vector<int> &field_ids,
                                  std::function<void(bool)> handler);
   void on_property_set(bool error,
                        const std::vector<Emodel_Property_Pair*> &properties_changed,
                        std::function<void(bool)> handler);

private:
   ::emc::database &database;
   esql::model_database &db;
   std::function<void(bool)> callback;

   std::unordered_set<std::string> pending_properties;
};

}

#endif
