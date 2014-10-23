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
         Eina_Value value_prop;
         Eo *child;
         unsigned int i = 0;
         EINA_ACCESSOR_FOREACH(_ac, i, child)
           {
              eo_do(child, emodel_property_get(TABLE_PROP_NAME, &value_prop));
              char *tablename = eina_value_to_string(&value_prop);
              if (strcmp(tablename, ARTISTIS_TABLE_NAME) == 0)
                  artists = esql::model_table(child);
              else if (strcmp(tablename, ALBUMS_TABLE_NAME) == 0)
                  albums = esql::model_table(child);
              else if (strcmp(tablename, TRACKS_TABLE_NAME) == 0)
                  tracks = esql::model_table(child);

              eina_value_flush(&value_prop);
              free(tablename);
           }
         is_load = true;
         loaded();

         return EINA_FALSE;
      }
     , std::placeholders::_3));
}

void
audiolistmodel::artists_get(std::function<void(efl::eo::base&)> func)
{
   if (is_load)
     {
        artists.load();
        func(artists);
     }
   else
     {
        loaded = std::bind([this](std::function<void(efl::eo::base&)> func)
                   {
                      artists.load();
                      func(artists);
                   }, func);
        database.load();
     }
}

void
audiolistmodel::albums_get(std::function<void(efl::eo::base&)> func)
{
   if (is_load)
     {
        albums.load();
        func(albums);
     }
   else
     {
        loaded = std::bind([this](std::function<void(efl::eo::base&)> func)
                   {
                      albums.load();
                      func(albums);
                   }, func);

        database.load();
     }
}

void
audiolistmodel::tracks_get(std::function<void(efl::eo::base&)> func)
{
   if (is_load)
     {
        tracks.load();
        func(tracks);
     }
   else
     {
        loaded = std::bind([this](std::function<void(efl::eo::base&)> func)
                   {
                      tracks.load();
                      func(tracks);
                   }, func);

        database.load();
     }
}

void
audiolistmodel::artist_albums_get(efl::eo::base &artist, std::function<void(efl::eo::base&)> func)
{
}

void
audiolistmodel::artist_tracks_get(efl::eo::base &artist, std::function<void(efl::eo::base&)> func)
{
}

void
audiolistmodel::albums_tracks_get(efl::eo::base &album, std::function<void(efl::eo::base&)> func)
{
}

} //emc
