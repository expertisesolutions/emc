/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */
#include "audiolistmodel.hh"

#include "database_schema.hh"
#include "emodel_helpers.hh"
#include "logger.hh"
#include "tag_processor.hh"

#include <eina_accessor.hh>

#include <functional>
#include <utility>

namespace {
   const auto INVALID_ID = 0;

   template<typename T, typename... Args>
   std::unique_ptr<T> make_unique(Args&&... args)
   {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
   }

   int64_t
   get_id(const std::unordered_map<std::string, esql::model_row> &map,
          const std::string &key)
   {
      int64_t id = INVALID_ID;

      auto it = map.find(key);
      if (end(map) != it)
        emc::emodel_helpers::property_get(it->second, "id", id);
      return id;

   }
}

namespace emc {

audiolistmodel::audiolistmodel(::emc::database &_database)
   : maps_ready(false)
   , loading_rows_count(0)
   , tag_reader(std::bind(&audiolistmodel::media_file_add_cb, this, std::placeholders::_1))
   , scanner(std::bind(&tag_reader::tag_file, &tag_reader, std::placeholders::_1))
   , database(_database)
{
   DBG << "Starting file scanner...";
   scanner.start();

   DBG << "Loading database...";
   database.async_load(std::bind(&audiolistmodel::on_database_loaded, this, std::placeholders::_1));
}

audiolistmodel::~audiolistmodel()
{
}

void
audiolistmodel::on_database_loaded(bool error)
{
   if (error)
     {
        ERR << "Error loading database";
        return;
     }

   populate_maps();
}

esql::model_table&
audiolistmodel::artists_get()
{
   auto &artists = database.artists_get();
   artists.filter_set("1=1 ORDER BY name");
   artists.load();
   DBG << artists.name_get();
   return artists;
}

esql::model_table&
audiolistmodel::albums_get()
{
   auto &albums = database.albums_get();
   albums.filter_set("");
   albums.load();
   return albums;
}

esql::model_table&
audiolistmodel::tracks_get()
{
   auto &tracks = database.tracks_get();
   tracks.filter_set("");
   tracks.load();
   return tracks;
}

esql::model_table&
audiolistmodel::artist_albums_get(esql::model_row& artist)
{
   auto &albums = database.albums_get();

   Eina_Value value;
   char *id_artist = NULL;

   artist.property_get("id", &value);
   id_artist = eina_value_to_string(&value);
   if (id_artist)
     {
        std::string f("id_artist=");
        albums.filter_set(f += id_artist);
        return albums;
     }
   albums.load();
   return albums;
}

esql::model_table&
audiolistmodel::artist_tracks_get(esql::model_row& artist)
{
    auto &tracks = database.tracks_get();

    Eina_Value value;
    char *id_artist = NULL;

    artist.property_get("id", &value);
    id_artist = eina_value_to_string(&value);
    if (id_artist)
      {
         std::string f("id_artist=");
         tracks.filter_set( f + id_artist);
      }
    tracks.load();
    return tracks;
}

esql::model_table&
audiolistmodel::album_tracks_get(esql::model_row& album)
{
    auto &tracks = database.tracks_get();

    Eina_Value value;
    char *id_album = NULL;

    album.property_get("id", &value);
    id_album = eina_value_to_string(&value);
    if (id_album)
      {
         std::string f("id_album=");
         tracks.filter_set(f + id_album + " ORDER BY track");
      }
    tracks.load();
    return tracks;
}

void
audiolistmodel::media_file_add_cb(const tag &tag)
{
   pending_tags.push(tag);

   if (maps_ready)
     process_pending_tags();
}

void
audiolistmodel::populate_maps()
{
   auto &artists = database.artists_get();
   auto &albums = database.albums_get();
   auto &tracks = database.tracks_get();

   populate_map(artists, "name", artist_map);
   populate_map(albums, "name", album_map);
   populate_map(tracks, "file", track_map);

   if (!loading_rows_count)
     {
        maps_ready = true;
        process_pending_tags();
     }
}

void
audiolistmodel::populate_map(esql::model_table &table, const std::string &key_field, std::unordered_map<std::string, esql::model_row> &map)
{
   DBG << "Populating map...";
   auto rows = emc::emodel_helpers::children_get<esql::model_row>(table);

   loading_rows_count += rows.size();

   for (auto &row : rows)
     {
        emc::emodel_helpers::async_properties_load(row, [this, row, key_field, &map](bool error)
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

             map.insert(std::make_pair(value, row));

             if (!loading_rows_count)
               {
                  maps_ready = true;
                  process_pending_tags();
               }
          });
     }
}

void
audiolistmodel::process_pending_tags()
{
   if (is_processing_tags())
     return;

   DBG << "Processing " << pending_tags.size() << " pending tags...";
   while (!is_processing_tags() && !pending_tags.empty())
     {
        tag tag = pending_tags.front();
        pending_tags.pop();
        process_tag(tag);
     }
}

void
audiolistmodel::process_tag(const tag &tag)
{
   DBG << "Processing tag: " << tag.file;
   auto &artists = database.artists_get();
   auto &albums = database.albums_get();
   auto &tracks = database.tracks_get();

   auto next_processor = std::bind(&audiolistmodel::next_processor, this);

   auto artist_processor = make_unique<tag_processor>(artist_map, artists, tag.artist, next_processor,
     [tag](esql::model_row row)
     {
        DBG << "Setting artist properties: " << tag.artist;
        emc::emodel_helpers::property_set(row, "name", tag.artist);
     });

   auto album_processor = make_unique<tag_processor>(album_map, albums, tag.album, next_processor,
     [tag, this](esql::model_row row)
     {
        DBG << "Setting album properties: " << tag.album;
        auto artist_id = artist_id_get(tag.artist);
        emc::emodel_helpers::property_set(row, "name", tag.album);
        if (INVALID_ID != artist_id)
          emc::emodel_helpers::property_set(row, "id_artist", artist_id);
        emc::emodel_helpers::property_set(row, "genre", tag.genre);
        emc::emodel_helpers::property_set(row, "year", tag.year);
     });

   auto track_processor = make_unique<tag_processor>(track_map, tracks, tag.file, next_processor,
     [tag, this](esql::model_row row)
     {
        DBG << "Setting track properties: " << tag.file;
        auto artist_id = artist_id_get(tag.artist);
        auto album_id = album_id_get(tag.album);
        emc::emodel_helpers::property_set(row, "file", tag.file);
        if (INVALID_ID != artist_id)
          emc::emodel_helpers::property_set(row, "id_artist", artist_id);
        if (INVALID_ID != album_id)
          emc::emodel_helpers::property_set(row, "id_album", album_id);
        emc::emodel_helpers::property_set(row, "name", tag.title);
        emc::emodel_helpers::property_set(row, "track", tag.track);
        emc::emodel_helpers::property_set(row, "artwork", tag.artwork);
     });

   processing_tags.push(move(artist_processor));
   processing_tags.push(move(album_processor));
   processing_tags.push(move(track_processor));
   process();
}

bool
audiolistmodel::process()
{
   DBG << "Processing " << processing_tags.size();
   while (!processing_tags.empty())
     {
        bool is_processing_pending = processing_tags.front()->process();
        if (is_processing_pending) return true;

        DBG << "Going to next processor";
        processing_tags.pop();
     }

   return false;
}

void
audiolistmodel::next_processor()
{
   if (!processing_tags.empty())
     {
        DBG << "Going to next processor";
        processing_tags.pop();
     }

   if (processing_tags.empty())
     {
        DBG << "No more processors";
        process_pending_tags();
        return;
     }

   bool is_processing_pending = process();
   if (is_processing_pending) return;
   process_pending_tags();
};

int64_t
audiolistmodel::artist_id_get(const std::string &artist_name) const
{
   return get_id(artist_map, artist_name);
}

int64_t
audiolistmodel::album_id_get(const std::string &album_name) const
{
   return get_id(album_map, album_name);
}

int64_t
audiolistmodel::track_id_get(const std::string &file_name) const
{
   return get_id(track_map, file_name);
}

bool audiolistmodel::is_processing_tags() const
{
   return !processing_tags.empty();
}

} //emc
