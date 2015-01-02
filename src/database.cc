#include "database.hh"

#include "database_schema.hh"
#include "emodel_helpers.hh"
#include "logger.hh"
#include "row_map.hh"

#include <cassert>
#include <ostream>

namespace emc {

const int database::INVALID_ID = 0;

database::database()
   : db(nullptr)
   , artists(nullptr)
   , albums(nullptr)
   , tracks(nullptr)
   , settings(nullptr)
   , version(nullptr)
   , checker(*this)
   , child_added_connection(nullptr)
{
}

database::~database()
{}

void
database::async_load(std::function<void(bool)> handler)
{
   DBG << "Loading database";
   if (handler)
      this->handlers.push_back(handler);
   db = esql::model_database(db.esql_model_database_constructor("./emc.db", "", "", ""));
   child_added_connection = emodel_helpers::child_added_event_connect(
     db, std::bind(&database::on_child_added, this, std::placeholders::_1));
   emc::emodel_helpers::async_load(db, std::bind(&database::on_database_loaded, this, std::placeholders::_1));
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
database::on_database_loaded(bool error)
{
  if (error)
    {
       ERR << "Error loading database";
       failure();
       return;
    }

  map_tables();

  emodel_helpers::async_children_load(db, std::bind(&database::on_tables_loaded, this, std::placeholders::_1));
}

void
database::on_tables_loaded(bool error)
{
  if (error)
    {
       ERR << "Error loading database tables";
       failure();
       return;
    }

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
   checker.async_check(std::bind(&database::on_database_checked, this, std::placeholders::_1));
}

void
database::on_database_checked(bool error)
{
  if (error)
    {
       ERR << "Error checking database schema";
       failure();
       return;
    }

  success();
}

void
database::on_child_added(const Emodel_Children_Event &event)
{
   esql::model_table table(::eo_ref(event.child));
   assert(table);
   const std::string tablename = table.name_get();
   DBG << "Table added: " << tablename;
   tables.insert(std::make_pair(tablename, table));
}

esql::model_database&
database::database_get() const
{
   return db;
}

esql::model_table&
database::artists_get() const
{
   if (!artists)
     artists = table_get(schema::artists_table.name);

   assert(artists && "Artists table not found or not loaded yet");
   return artists;
}

esql::model_table&
database::albums_get() const
{
   if (!albums)
     albums = table_get(schema::albums_table.name);

   assert(albums && "Albums table not found or not loaded yet");
   return albums;
}

esql::model_table&
database::tracks_get() const
{
   if (!tracks)
     tracks = table_get(schema::tracks_table.name);

   assert(tracks && "Tracks table not found or not loaded yet");
   return tracks;
}

esql::model_table&
database::settings_get() const
{
   if (!settings)
     settings = table_get(schema::settings_table.name);

   assert(settings && "Settings table not found or not loaded yet");
   return settings;
}

esql::model_table&
database::version_get() const
{
   if (!version)
     version = table_get(schema::version_table.name);

   assert(version && "Version table not found or not loaded yet");
   return version;
}

esql::model_table
database::table_get(const std::string &table) const
{
   auto it = tables.find(table);
   if (end(tables) == it)
     return esql::model_table(nullptr);
   return it->second;
}

bool
database::is_empty() const
{
   return tables.empty();
}

}
