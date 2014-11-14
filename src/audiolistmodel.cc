/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include "audiolistmodel.hh"

#define ARTISTIS_TABLE_NAME "artists"
#define ALBUMS_TABLE_NAME "albums"
#define TRACKS_TABLE_NAME "tracks"

#define TABLE_PROP_NAME "table"

namespace emc {

audiolistmodel::audiolistmodel()
    :   database(nullptr),
        artists(nullptr),
        albums(nullptr),
        tracks(nullptr),
        is_load(false)
{
   esql_init();
   database = esql::model("./emc.db", "", "", "");

   database.callback_children_count_changed_add(std::bind([this](void * info)
      {
         unsigned int len = *(unsigned int *)info;
         std::cout << "audioDB children count change " << len << std::endl;
         if (len == 0) return EINA_TRUE;

         if (is_load) return EINA_FALSE;

         Eina_Accessor *_ac = NULL;
         database.children_slice_get(0, 0, &_ac);
         if (_ac == NULL) return EINA_FALSE;
        /*
         efl::eina::accessor<efl::eo::base> a(_ac);
         for (efl::eo::base c : a)
           {}
        */

         /* FIXME XXX USE EINA-CXX */
         Eo *child;
         unsigned int i = 0;
         EINA_ACCESSOR_FOREACH(_ac, i, child)
           {
              esql::model_table table(child);
              std::string tablename = table.name_get();

              if (tablename == ARTISTIS_TABLE_NAME)
                  artists = table;
              else if (tablename == ALBUMS_TABLE_NAME)
                  albums = table;
              else if (tablename == TRACKS_TABLE_NAME)
                  tracks = table;
           }

         is_load = true;
         return EINA_FALSE;
      }, std::placeholders::_3));

   database.load();
}

esql::model_table&
audiolistmodel::artists_get()
{
    artists.filter_set("");
    artists.load();
    std::cout << artists.name_get() << std::endl;
    return artists;
}

esql::model_table&
audiolistmodel::albums_get()
{
    albums.filter_set("");
    albums.load();
    return albums;
}

esql::model_table&
audiolistmodel::tracks_get()
{
    tracks.filter_set("");
    tracks.load();
    return tracks;
}

esql::model_table&
audiolistmodel::artist_albums_get(esql::model_row& artist)
{
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

} //emc
