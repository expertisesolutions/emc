/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */
#include "audiolistmodel.hh"

#include "database_schema.hh"
#include "emodel_helpers.hh"
#include "logger.hh"
#include "tag_processor.hh"

#include <Ecore.hh>

#include <functional>
#include <sstream>
#include <utility>

namespace {
   const auto INVALID_ID = 0;

   template<typename T, typename... Args>
   std::unique_ptr<T> make_unique(Args&&... args)
   {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
   }

   int64_t
   get_id(esql::model_row &row)
   {
      int64_t id = INVALID_ID;
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

audiolistmodel::audiolistmodel(::emc::database &_database)
   : database_map(database)
   , tag_reader(std::bind(&audiolistmodel::tag_read_cb, this, std::placeholders::_1))
   , scanner(std::bind(&tag_reader::tag_file, &tag_reader, std::placeholders::_1))
   , tag_pool(5)
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

   database_map.async_map(std::bind(&audiolistmodel::on_rows_mapped, this));
}

void
audiolistmodel::on_rows_mapped()
{
   process_pending_tags();
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

   auto id_artist = get_id(artist);
   if (INVALID_ID != id_artist)
     {
        std::stringstream buffer;
        buffer << "id_artist=" << id_artist;
        albums.filter_set(buffer.str());
        return albums;
     }

   albums.load();
   return albums;
}

esql::model_table&
audiolistmodel::artist_tracks_get(esql::model_row& artist)
{
    auto &tracks = database.tracks_get();

    auto id_artist = get_id(artist);
    if (INVALID_ID != id_artist)
      {
         std::stringstream buffer;
         buffer << "id_artist=" << id_artist;
         tracks.filter_set(buffer.str());
      }

    tracks.load();
    return tracks;
}

esql::model_table&
audiolistmodel::album_tracks_get(esql::model_row& album)
{
    auto &tracks = database.tracks_get();

    auto id_album = get_id(album);
    if (INVALID_ID != id_album)
      {
         std::stringstream buffer;
         buffer << "id_album="
             << id_album
             << " ORDER BY track";
         tracks.filter_set(buffer.str());
      }

    tracks.load();
    return tracks;
}

void
audiolistmodel::tag_read_cb(const tag &tag)
{
   tag_pool.add(tag);
   efl::ecore::main_loop_thread_safe_call_async(std::bind(&audiolistmodel::media_file_add_cb, this, tag));
}

void
audiolistmodel::media_file_add_cb(const tag &tag)
{
   pending_tags.push(tag);

   if (database_map.is_mapped())
     process_pending_tags();
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

   auto next_processor =
   [this, tag]()
   {
      efl::ecore::main_loop_thread_safe_call_async(std::bind(&audiolistmodel::next_processor, this, tag));
   };

   auto artist_processor = make_unique<tag_processor>(database_map.artists_map_get(), artists, tag.artist, next_processor,
     [tag](esql::model_row row)
     {
        DBG << "Setting artist properties: " << tag.artist;
        emc::emodel_helpers::property_set(row, "name", tag.artist);
     });

   auto album_processor = make_unique<tag_processor>(database_map.albums_map_get(), albums, tag.album, next_processor,
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

   auto track_processor = make_unique<tag_processor>(database_map.tracks_map_get(), tracks, tag.file, next_processor,
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

   bool is_processing_pending = process();
   if (is_processing_pending) return;

   DBG << "No more processors";
   tag_pool.remove(tag);
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
audiolistmodel::next_processor(const tag &tag)
{
   if (!processing_tags.empty())
     {
        DBG << "Going to next processor";
        processing_tags.pop();
     }

   bool is_processing_pending = process();
   if (is_processing_pending) return;

   DBG << "No more processors";
   tag_pool.remove(tag);

   process_pending_tags();
};

int64_t
audiolistmodel::artist_id_get(const std::string &artist_name) const
{
   return get_id(database_map.artists_map_get(), artist_name);
}

int64_t
audiolistmodel::album_id_get(const std::string &album_name) const
{
   return get_id(database_map.albums_map_get(), album_name);
}

int64_t
audiolistmodel::track_id_get(const std::string &file_name) const
{
   return get_id(database_map.tracks_map_get(), file_name);
}

bool audiolistmodel::is_processing_tags() const
{
   return !processing_tags.empty();
}

} //emc
