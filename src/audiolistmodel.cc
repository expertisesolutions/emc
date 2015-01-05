/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */
#include "audiolistmodel.hh"

#include "database.hh"
#include "database_schema.hh"
#include "emodel_helpers.hh"
#include "logger.hh"
#include "tag_processor.hh"

#include <Ecore.hh>

#include <functional>
#include <sstream>
#include <utility>

namespace {
   int64_t
   get_id(esql::model_row &row)
   {
      int64_t id = emc::database::INVALID_ID;
      if (row)
        emc::emodel_helpers::property_get(row, "id", id);
      return id;
   }
}

namespace emc {

audiolistmodel::audiolistmodel(::emc::database &database)
   : database(database)
{}

audiolistmodel::~audiolistmodel()
{}

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
   if (database::INVALID_ID != id_artist)
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
    if (database::INVALID_ID != id_artist)
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
    if (database::INVALID_ID != id_album)
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
audiolistmodel::artist_get(esql::model_row& row, std::function<void(esql::model_row&)> handler)
{
   auto &artists = database.artists_get();
   int64_t id_artist = emc::database::INVALID_ID;

   emodel_helpers::property_get(row, "id_artist", id_artist);

   if (database::INVALID_ID != id_artist)
     {
         std::stringstream buffer;
         buffer << "id="
             << id_artist;
         auto connection = std::make_shared<::efl::eo::signal_connection>([]{});
         artists.filter_set(buffer.str());
         *connection = artists.callback_load_status_add(
               std::bind([connection, handler, &artists](void *info)
                 {
                     const Emodel_Load &st = *static_cast<Emodel_Load*>(info);
                     if (st.status & EMODEL_LOAD_STATUS_LOADED)
                       {
                            connection->disconnect();
                            auto children = emodel_helpers::children_get<esql::model_row>(artists);
                            if (children.empty())
                                return;

                            emodel_helpers::async_properties_load(children[0], std::bind(handler, children[0]));
                       }
                 }, std::placeholders::_3));
     }
}

}
