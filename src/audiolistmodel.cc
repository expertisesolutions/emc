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

audiolistmodel::audiolistmodel(::emc::database &database)
   : database_map(database)
   , tag_reader(std::bind(&audiolistmodel::on_tag_read, this, std::placeholders::_1))
   , scanner(std::bind(&tag_reader::tag_file, &tag_reader, std::placeholders::_1))
   , tag_pool(5)
   , database(database)
   , updater(database, database_map, std::bind(&audiolistmodel::on_tag_updated, this, std::placeholders::_1))
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
audiolistmodel::on_tag_read(const tag &tag)
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
   while (!updater.is_updating() && !pending_tags.empty())
     {
        auto &tag = pending_tags.front();
        updater.update(tag);
     }
}

void
audiolistmodel::on_tag_updated(const tag &tag)
{
   tag_pool.remove(tag);
   pending_tags.pop();
}

} //emc
