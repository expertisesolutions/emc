/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include "audiolistmodel.hh"

#include <eina_accessor.hh>

#include "database_schema.hh"

namespace emc {

audiolistmodel::audiolistmodel()
    :   database(nullptr),
        artists(nullptr),
        albums(nullptr),
        tracks(nullptr),
        init_connection(nullptr),
        db_table_created_connection(nullptr)
{
   // TODO: Configure database path to user data
   database = esql::model(database.esql_model_constructor("./emc.db", "", "", ""));

   init_connection = database.callback_children_count_changed_add(
     std::bind(&audiolistmodel::init, this, std::placeholders::_3));

   database.load();
}

audiolistmodel::~audiolistmodel()
{
   init_connection.disconnect();
}

bool
audiolistmodel::init(void * info)
{
   init_connection.disconnect();

   // TODO: Create a service to take care of database schema creation/validation/migration

   unsigned int table_count = *static_cast<unsigned int *>(info);
   std::cout << "audioDB children count change " << table_count << std::endl;
   if (0 == table_count)
     {
        std::cout << "Creating database..." << std::endl;
        db_table_created_connection = database.callback_children_count_changed_add(
          std::bind(&audiolistmodel::db_table_created, this, std::placeholders::_3));
        // TODO: connect the load_status_error callback
        schema::create_database(database);
        return false;
     }


   // TODO: Validate schema version
   // if (schema::tables.size() != table_count ...)

   // TODO: Migrate old schema version or error on newer than current
   // if (version_table.value != current_version)

   load_tables();
   return false;
}

bool
audiolistmodel::db_table_created(void * info)
{
   unsigned int table_count = *static_cast<unsigned int *>(info);
   std::cout << "Database table created. Number of tables: " << table_count << std::endl;
   if (schema::tables.size() != table_count)
     return false;

   db_table_created_connection.disconnect();

   load_tables();
   return false;
}

bool
audiolistmodel::load_tables()
{
   Eina_Accessor *_ac = nullptr;
   database.children_slice_get(0, 0, &_ac);
   if (nullptr == _ac) return false;

   // FIXME: Use EINA-CXX
   Eo *child;
   unsigned int i = 0;
   EINA_ACCESSOR_FOREACH(_ac, i, child)
     {
        esql::model_table table(::eo_ref(child));

        std::string tablename = table.name_get();

        if (schema::artists_table.name == tablename)
          artists = table;
        else if (schema::albums_table.name == tablename)
          albums = table;
        else if (schema::tracks_table.name == tablename)
          tracks = table;
     }

   std::cout << "Starting file scanner..." << std::endl;
   scanner.start();

   // TODO: Check new/old files and tracks against db/filesystem

   return false;
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
