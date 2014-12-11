/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include "audiolistmodel.hh"

#include <functional>
#include <utility>

#include <eina_accessor.hh>

#include "database_schema.hh"
#include "tag_processor.hh"
#include "emodel_helpers.hh"

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

audiolistmodel::audiolistmodel()
   : scanner(std::bind(&audiolistmodel::media_file_add_cb, this, std::placeholders::_1))
   , maps_ready(false)
   , loading_rows_count(0)
{
   std::cout << "Starting file scanner..." << std::endl;
   scanner.start();

   std::cout << "Loading database..." << std::endl;
   database.async_load(std::bind(&audiolistmodel::on_database_loaded, this, std::placeholders::_1));
}

audiolistmodel::~audiolistmodel()
{
}

bool
audiolistmodel::on_database_loaded(bool error)
{
   if (error)
     {
        std::cerr << " Error loading database" << std::endl;
        return false;
     }

   populate_maps();
   return false;
}

esql::model_table&
audiolistmodel::artists_get()
{
   auto &artists = database.artists_get();
   artists.filter_set("");
   artists.load();
   std::cout << artists.name_get() << std::endl;
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
   using namespace std;
   cout << "file - \"" << tag.file << "\"" << endl;
   //cout << "title - \"" << tag.title << "\"" << endl;
   //cout << "artist - \"" << tag.artist << "\"" << endl;
   //cout << "album - \"" << tag.album << "\"" << endl;
   //cout << "year - \"" << tag.year << "\"" << endl;
   //cout << "track - \"" << tag.track << "\"" << endl;
   //cout << "genre - \"" << tag.genre << "\"" << endl;

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
   std::cout << "Populating map..." << std::endl;
   auto rows = emc::emodel_helpers::children_get<esql::model_row>(table);

   loading_rows_count += rows.size();

   for (auto &row : rows)
     {
        emc::emodel_helpers::async_properties_load(row, [this, row, key_field, &map](bool error)
          {
             --loading_rows_count;

             if (error)
               {
                  std::cout << "Error loading row" << std::endl;
                  return;
               }

             std::string value;
             if (!emc::emodel_helpers::property_get(row, key_field, value))
               {
                  std::cout << "Error property_get('" << key_field << "')" << std::endl;
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

   std::cout << "Processing " << pending_tags.size() << " pending tags..." << std::endl;
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
   std::cout << "Processing tag: " << tag.file << std::endl;
   auto &artists = database.artists_get();
   auto &albums = database.albums_get();
   auto &tracks = database.tracks_get();

   auto next_processor = std::bind(&audiolistmodel::next_processor, this);

   auto artist_processor = make_unique<tag_processor>(artist_map, artists, tag.artist, next_processor,
     [tag](esql::model_row row)
     {
        std::cout << "Setting artist properties: " << tag.artist << std::endl;
        emc::emodel_helpers::property_set(row, "name", tag.artist);
     });

   auto album_processor = make_unique<tag_processor>(album_map, albums, tag.album, next_processor,
     [tag, this](esql::model_row row)
     {
        std::cout << "Setting album properties: " << tag.album << std::endl;
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
        std::cout << "Setting track properties: " << tag.file << std::endl;
        auto artist_id = artist_id_get(tag.artist);
        auto album_id = album_id_get(tag.album);
        emc::emodel_helpers::property_set(row, "file", tag.file);
        if (INVALID_ID != artist_id)
          emc::emodel_helpers::property_set(row, "id_artist", artist_id);
        if (INVALID_ID != album_id)
          emc::emodel_helpers::property_set(row, "id_album", album_id);
        emc::emodel_helpers::property_set(row, "name", tag.title);
        emc::emodel_helpers::property_set(row, "track", tag.track);
     });

   processing_tags.push(move(artist_processor));
   processing_tags.push(move(album_processor));
   processing_tags.push(move(track_processor));
   std::cout << "Processing " << processing_tags.size() << std::endl;
   processing_tags.front()->process();
}

void
audiolistmodel::next_processor()
{
   std::cout << this << std::endl;
   if (!processing_tags.empty())
     {
        std::cout << "Going to next processor" << std::endl;
        processing_tags.pop();
     }

   if (processing_tags.empty())
     {
        std::cout << "No more processors" << std::endl;
        process_pending_tags();
        return;
     }


   std::cout << "Processing " << processing_tags.size() << std::endl;
   processing_tags.front()->process();
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
