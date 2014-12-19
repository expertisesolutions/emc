#include "database.hh"

#include "database_schema.hh"
#include "emodel_helpers.hh"
#include "logger.hh"

#include <cassert>
#include <ostream>

namespace emc {

database::database()
   : db(nullptr)
   , artists(nullptr)
   , albums(nullptr)
   , tracks(nullptr)
   , settings(nullptr)
   , loading_tables_count(0)
{}

database::~database()
{}

void
database::async_load(std::function<void(bool)> handler)
{
   DBG << "Loading database";
   if (handler)
      this->handlers.push_back(handler);
   db = esql::model_database(db.esql_model_database_constructor("./emc.db", "", "", ""));
   emc::emodel_helpers::async_load(db, std::bind(&database::on_load, this, std::placeholders::_1));
}

void
database::load_callback_add(std::function<void(bool)> handler)
{
   if (handler)
      this->handlers.push_back(handler);
}

void
database::async_create_row(esql::model_table &table, std::function<void(bool, esql::model_row)> callback)
{
   auto obj = table.child_add();
   DBG << "Row created: " << obj._eo_ptr();
   esql::model_row row(::eo_ref(obj._eo_ptr()));
   emc::emodel_helpers::async_properties_load(row, std::bind(callback, std::placeholders::_1, row));
}

void
database::success()
{
   notify(false);
}

void
database::failure()
{
   notify(true);
}

void
database::notify(bool error)
{
   for (auto &handler : handlers)
     {
        if (handler) {
           handler(error);
        }
     }
   handlers.clear();
}

void
database::on_load(bool error)
{
  if (error)
    {
       ERR << "Error loading database";
       failure();
       return;
    }

  map_tables();
  check_tables();
}

void
database::map_tables()
{
   DBG << "Mapping tables";
   auto tables = emc::emodel_helpers::children_get<esql::model_table>(db);
   for (auto &table : tables)
     {
        const std::string tablename = table.name_get();
        this->tables.insert(std::make_pair(tablename, table));
     }
}

void
database::check_tables()
{
   DBG << "Checking tables";

   // version 0 is an empty database
   if (tables.empty())
     {
        migrate(0);
        return;
     }

   load_tables();
}

void
database::migrate(int version)
{
   DBG << "Migrating version from " << version;

   switch(version)
     {
      case 0:
        {
           migrate_from_version_0();
           return;
        }
      case emc::schema::v1::VERSION:
        {
           migrate_from_version_1();
           return;
        }
      case emc::schema::CURRENT_VERSION:
        {
           DBG << "Version " << version << " is the current one. Done!";
           success();
           return;
        }
      default:
        {
           ERR << "Migration procedure not defined";
           failure();
           return;
        }
     }
}

void
database::migrate_from_version_0()
{
   DBG << "Migrating from version 0";

   std::function<void()> migration_done = std::bind(&database::set_version, this, schema::CURRENT_VERSION);

   emc::emodel_helpers::callback_children_count_changed_add(db,
     std::bind(&database::on_table_created, this, std::placeholders::_1, std::placeholders::_2, schema::tables.size(), migration_done));

   for (auto &table : schema::tables)
     create_table(*table);
}

void
database::create_table(const schema::table &table_definition)
{
   DBG << "Creating table: " << table_definition.name;
   efl::eo::base obj = db.child_add();
   esql::model_table table(::eo_ref(obj._eo_ptr()));
   table.name_set(table_definition.name);
   tables.insert(std::make_pair(table_definition.name, table));

   for (auto &field : table_definition.fields)
     create_table_field(table, table_definition, field);
}

void
database::create_table_field(esql::model_table table, const schema::table &table_definition, const schema::field &field_definition)
{
   DBG << "Creating field: " << table_definition.name << "." << field_definition.name;
   ::efl::eina::value field_type(field_definition.type + " " + field_definition.constraint);
   table.property_set(field_definition.name, *field_type.native_handle());
}

bool
database::on_table_created(bool error, unsigned int actual_count, size_t expected_count, std::function<void()> migration_done)
{
   if (error)
     {
        ERR << "Error creating table";
        failure();
        return false;
     }

   if (actual_count != expected_count)
     return true;

   migration_done();
   return false;
}

void
database::set_version(int version)
{
   DBG << "Setting version to " << version;

   auto it = tables.find(emc::schema::version_table.name);
   assert(end(tables) != it);
   auto &version_table = it->second;

   emc::emodel_helpers::async_load(version_table, std::bind(&database::on_version_table_loaded, this, std::placeholders::_1, version_table, version));
}

void
database::on_version_table_loaded(bool error, esql::model_table version_table, int version)
{
   if (error)
     {
        ERR << "Error loading version table";
        failure();
        return;
     }

   auto rows = emc::emodel_helpers::children_get<esql::model_row>(version_table);
   assert(rows.size() <= 1);

   if (rows.empty())
     async_create_row(version_table, std::bind(&database::on_version_table_row_loaded, this, std::placeholders::_1, std::placeholders::_2, version));
   else
     load_version_row(false, rows.front(), version);
}

void
database::load_version_row(bool error, esql::model_row row, int version)
{
   if (error)
     {
        ERR << "Error creating version table row";
        failure();
        return;
     }

   DBG << "Loading version row";
   emc::emodel_helpers::async_load(row, std::bind(&database::on_version_table_row_loaded, this, std::placeholders::_1, row, version));
}

void
database::on_version_table_row_loaded(bool error, esql::model_row row, int version)
{
   if (error)
     {
        ERR << "Error loading version table row";
        failure();
        return;
     }

   DBG << "Setting version row";
   auto callback = std::bind(&database::on_version_table_row_setted, this, std::placeholders::_1, row, version);
   emc::emodel_helpers::callback_properties_changed_once(row, callback);
   emc::emodel_helpers::property_set(row, "version", version);
}

void
database::on_version_table_row_setted(bool error, esql::model_row row, int version)
{
   if (error)
     {
        ERR << "Error setting version on table row";
        failure();
        return;
     }

   migrate(version);
}

void
database::load_tables()
{
   loading_tables_count = tables.size();
   DBG << "Loading " << loading_tables_count << " tables";

   for (auto &name2table : tables)
     {
        auto &table = name2table.second;
        emc::emodel_helpers::async_load(table, std::bind(&database::on_table_loaded, this, std::placeholders::_1));
     }
}

void
database::on_table_loaded(bool error)
{
   --loading_tables_count;

   if (error)
     {
        ERR << "Error loading table";
        failure();
        return;
     }

   if (loading_tables_count) return;

   get_version();
}

void
database::get_version()
{
   DBG << "Getting database schema version";

   auto it = tables.find(emc::schema::version_table.name);
   if (end(tables) == it)
     {
        migrate(emc::schema::v1::VERSION);
        return;
     }

   auto &version_table = it->second;
   auto rows = emc::emodel_helpers::children_get<esql::model_row>(version_table);
   assert(!rows.empty());
   auto row = rows.front();
   emc::emodel_helpers::async_load(row, std::bind(&database::on_version_row_loaded, this, std::placeholders::_1, row));
}

void
database::on_version_row_loaded(bool error, esql::model_row row)
{
   int64_t version;
   if (!emc::emodel_helpers::property_get(row, "version", version))
     {
        ERR << "Error getting database schema version";
        failure();
        return;
     }

   migrate(version);
}

void
database::migrate_from_version_1()
{
   DBG << "Migrating from version 1 to 2";

   for (auto &table : schema::v1::tables)
     {
        auto it = tables.find(table->name);
        assert(end(tables) != it);
     }


   auto it = tables.find(schema::v2::tracks_table.name);
   assert(end(tables) != it);
   auto &tracks_table = it->second;

   auto callback = [this](bool error)
     {
        if (error)
          {
             ERR << "Error creating field";
             failure();
             return;
          }

        create_version_table();
     };
   emc::emodel_helpers::callback_properties_changed_once(tracks_table, callback);
   create_table_field(tracks_table, schema::v2::tracks_table, schema::v2::tracks_table.fields[6]);
}

void
database::create_version_table()
{
   DBG << "Creating version table";

   emc::emodel_helpers::callback_children_count_changed_add(db,
     std::bind(&database::on_version_table_created, this, std::placeholders::_1));
   create_table(schema::v2::version_table);
}

bool
database::on_version_table_created(bool error)
{
   if (error)
     {
        ERR << "Error creating version table";
        failure();
        return false;
     }

   set_version(schema::v2::VERSION);
   return false;
}

esql::model_table& database::artists_get()
{
   if (!artists)
     {
        auto it = tables.find(schema::artists_table.name);
        if (end(tables) != it)
          artists = it->second;
        else
          assert(false && "Artists table not found or not loaded yet");
     }

   return artists;
}

esql::model_table& database::albums_get()
{
   if (!albums)
     {
        auto it = tables.find(schema::albums_table.name);
        if (end(tables) != it)
          albums = it->second;
        else
          assert(false && "Artists table not found or not loaded yet");
     }

   return albums;
}

esql::model_table& database::tracks_get()
{
   if (!tracks)
     {
        auto it = tables.find(schema::tracks_table.name);
        if (end(tables) != it)
          tracks = it->second;
        else
          assert(false && "Artists table not found or not loaded yet");
     }

   return tracks;
}

esql::model_table& database::settings_get()
{
   if (!settings)
     {
        auto it = tables.find(schema::settings_table.name);
        if (end(tables) != it)
          settings = it->second;
        else
          assert(false && "Settings table not found or not loaded yet");
     }

   return settings;
}

}
