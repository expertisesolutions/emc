#include "database_map.hh"

#include "database.hh"
#include "emodel_helpers.hh"

namespace {
   int64_t
   get_id(esql::model_row &row)
   {
      int64_t id = emc::database::INVALID_ID;
      if (row)
        emc::emodel_helpers::property_get(row, "id", id);
      return id;
   }

   int64_t
   get_id(const emc::row_map &map,
          const std::string &key)
   {
      auto row = map.find(key);
      return get_id(row);
   }
}

namespace emc {

database_map::database_map(database &db)
   : db(db)
   , loading_rows_count(0)
   , mapped(false)
{}

database_map::~database_map()
{}

void database_map::async_map(std::function<void()> handler)
{
   this->handler = handler;

   auto &artists = db.artists_get();
   auto &albums = db.albums_get();
   auto &tracks = db.tracks_get();
   auto &settings = db.settings_get();

   populate_map(artists, "name", artists_map);
   populate_map(albums, "name", albums_map);
   populate_map(tracks, "file", tracks_map);
   populate_map(settings, "key", settings_map);

   notify_if_finished();
}

void
database_map::populate_map(esql::model_table &table, const std::string &key_field, row_map &map)
{
   DBG << "Populating map...";
   auto rows = emc::emodel_helpers::children_get<esql::model_row>(table);

   loading_rows_count += rows.size();

   for (auto &row : rows)
     {
        auto on_load = std::bind(&database_map::on_read_row, this, std::placeholders::_1, row, key_field, std::ref(map));
        emc::emodel_helpers::async_properties_load(row, on_load);
     }
}

void
database_map::on_read_row(bool error, esql::model_row row, const std::string &key_field, row_map &map)
{
   --loading_rows_count;

   if (error)
     {
        ERR << "Error loading row";
        return;
     }

   std::string value;
   if (!emc::emodel_helpers::property_get(row, key_field, value))
     {
        ERR << "Error property_get('" << key_field << "')";
        return;
     }

   map.add(value, row);
   notify_if_finished();
}

void
database_map::notify_if_finished()
{
   if (loading_rows_count > 0)
     return;

   mapped = true;
   handler();
}

bool
database_map::is_mapped() const
{
   return mapped;
}

row_map&
database_map::artists_map_get()
{
   return artists_map;
}

const row_map&
database_map::artists_map_get() const
{
   return artists_map;
}

row_map&
database_map::albums_map_get()
{
   return albums_map;
}

const row_map&
database_map::albums_map_get() const
{
   return albums_map;
}

row_map&
database_map::tracks_map_get()
{
   return tracks_map;
}

const row_map&
database_map::tracks_map_get() const
{
   return tracks_map;
}

int64_t
database_map::artist_id_get(const std::string &artist_name) const
{
   return get_id(artists_map, artist_name);
}

int64_t
database_map::album_id_get(const std::string &album_name) const
{
   return get_id(albums_map, album_name);
}

int64_t
database_map::track_id_get(const std::string &file_name) const
{
   return get_id(tracks_map, file_name);
}

std::string
database_map::setting_get(const std::string &key) const
{
   std::string value;

   auto row = settings_map.find(key);
   if (row)
     emc::emodel_helpers::property_get(row, "value", value);

   return value;
}

void
database_map::setting_set(const std::string &key, const std::string &value)
{
   auto row = settings_map.find(key);
   if (row)
     {
        emodel_helpers::property_set(row, "value", value);
        return;
     }

   auto &table = db.settings_get();
   db.async_create_row(table,
     [this, key, value](bool err, esql::model_row row)
     {
        emodel_helpers::property_set(row, "key", key);
        emodel_helpers::property_set(row, "value", value);
        settings_map.add(key, row);
     });
}

}
