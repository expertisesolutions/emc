/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include "audiolistmodel.hh"

#include <functional>
#include <utility>

#include <eina_accessor.hh>

#include "database_schema.hh"

namespace {
   template<typename T, typename... Args>
   std::unique_ptr<T> make_unique(Args&&... args)
   {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
   }

   template<class T>
   bool get_property(const ::emodel &model, const std::string &property, T &value)
   {
      ::efl::eina::value property_value;
      if (!model.property_get(property, property_value.native_handle()))
        {
           std::cout << "Error trying to get " << property << " property" << std::endl;
           return false;
        }

      value = ::efl::eina::get<T>(property_value);
      return true;
   }

   template<>
   bool get_property<int64_t>(const ::emodel &model, const std::string &property, int64_t &value)
   {
      ::efl::eina::value property_value;
      if (!model.property_get(property, property_value.native_handle()))
        {
           std::cout << "Error trying to get " << property << " property" << std::endl;
           return false;
        }

      eina_value_get(property_value.native_handle(), &value);
      return true;
   }
}

namespace emc {

audiolistmodel::audiolistmodel()
    :   database(nullptr),
        artists(nullptr),
        albums(nullptr),
        tracks(nullptr),
        init_connection(nullptr),
        db_table_created_connection(nullptr),
        scanner(std::bind(&audiolistmodel::media_file_add_cb, this, std::placeholders::_1)),
        maps_ready(false),
        processing_tag(false)
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

   populate_maps();

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
   populate_map(artists, "name", artist_map);
   populate_map(albums, "name", album_map);
   populate_map(tracks, "file", track_map);

   maps_ready = true;
   process_pending_tags();
}

void
audiolistmodel::populate_map(const esql::model_table &table, const std::string &key, std::unordered_map<std::string, esql::model_row> &map)
{
   std::cout << "Populating map..." << std::endl;
   Eina_Accessor *_ac = nullptr;
   table.children_slice_get(0, 0, &_ac);
   if (nullptr == _ac) return;

   // FIXME: Use EINA-CXX
   Eo *child;
   unsigned int i = 0;
   EINA_ACCESSOR_FOREACH(_ac, i, child)
     {
        esql::model_row row(::eo_ref(child));

        std::string value;
        if (!get_property(table, key, value)) continue;
        map.insert(std::make_pair(value, row));
     }
}

void
audiolistmodel::process_pending_tags()
{
   if (processing_tag)
     return;

   std::cout << "Processing " << pending_tags.size() << " pending tags..." << std::endl;
   while (!processing_tag && !pending_tags.empty())
     {
        tag tag = pending_tags.front();
        pending_tags.pop();
        processing_tag = process_tag(tag);
     }
}

bool
audiolistmodel::process_tag(const tag &tag)
{
   std::cout << "Processing tag..." << std::endl;
   return check_artist(tag) || check_album(tag) || check_track(tag);
}

bool
audiolistmodel::check_artist(const tag &tag)
{
   std::cout << "Checking artist: " << tag.artist << std::endl;
   if (tag.artist.empty())
     return false;

   auto it = artist_map.find(tag.artist);
   if (end(artist_map) != it)
     {
        std::cout << "Artist found: " << tag.artist << std::endl;
        return false;
     }

   std::cout << "Artist not found, creating artist and postponing album and track: " << tag.artist << std::endl;
   // create artist and postopone album/track
   auto obj = artists.child_add();
   esql::model_row row(::eo_ref(obj._eo_ptr()));
   auto connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   *connection = row.callback_load_status_add(std::bind(&audiolistmodel::new_artist_row_properties_loaded, this, connection, tag, row, std::placeholders::_3));
   row.properties_load();
   return true;
}

bool
audiolistmodel::new_artist_row_properties_loaded(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void *info)
{
   const Emodel_Load &st = *static_cast<Emodel_Load*>(info);
   if (st.status & EMODEL_LOAD_STATUS_ERROR)
     {
        connection->disconnect();
        std::cout << "Error loading new artist row properties: " << tag.artist << std::endl;
        return false;
     }

   if(!(st.status & EMODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return true;

   connection->disconnect();

   std::cout << "New artist ready to set values: " << tag.artist << std::endl;
   // TODO: Error callback
   auto new_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   *new_connection = row.callback_properties_changed_add(std::bind(&audiolistmodel::artist_row_inserted, this, new_connection, tag, row, std::placeholders::_3));
   ::efl::eina::value name(tag.artist);
   row.property_set("name", *name.native_handle());
   return false;
}

bool
audiolistmodel::artist_row_inserted(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void *info)
{
   std::cout << "New artist values have been set: " << tag.artist << std::endl;
   connection->disconnect();

   artist_map.insert(std::make_pair(tag.artist, row));

   processing_tag = check_album(tag) || check_track(tag);
   process_pending_tags();
   return false;
}

bool
audiolistmodel::check_album(const tag &tag)
{
   std::cout << "Checking album: " << tag.album << std::endl;
   if (tag.album.empty())
     return false;

   auto it = album_map.find(tag.album);
   if (end(album_map) != it)
     {
        std::cout << "Album found: " << tag.album << std::endl;
        return false;
     }

   std::cout << "Album not found, creating album and postponing track: " << tag.album << std::endl;
   // create album and postopone album/track
   auto obj = albums.child_add();
   esql::model_row row(::eo_ref(obj._eo_ptr()));
   auto connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   *connection = row.callback_load_status_add(std::bind(&audiolistmodel::new_album_row_properties_loaded, this, connection, tag, row, std::placeholders::_3));
   row.properties_load();
   return true;
}

bool
audiolistmodel::new_album_row_properties_loaded(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void *info)
{
   const Emodel_Load &st = *static_cast<Emodel_Load*>(info);

   if (st.status & EMODEL_LOAD_STATUS_ERROR)
     {
        connection->disconnect();
        std::cout << "Error loading new album row properties: " << tag.album << std::endl;
        return false;
     }

   if(!(st.status & EMODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return true;

   connection->disconnect();

   int64_t id_artist = 0;
   auto it = artist_map.find(tag.artist);
   if (end(artist_map) != it)
     {
        std::cout << "Getting artist id for: " << tag.artist << std::endl;
        get_property(it->second, "id", id_artist);
        std::cout << "Artist id=" << id_artist << std::endl;
     }

   std::cout << "New album ready to set values: " << tag.album << std::endl;
   auto new_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   *new_connection = row.callback_properties_changed_add(std::bind(&audiolistmodel::album_row_inserted, this, new_connection, tag, row, std::placeholders::_3));
   ::efl::eina::value name(tag.album);
   row.property_set("name", *name.native_handle());
   if (id_artist > 0)
     {
        ::efl::eina::value artist(id_artist);
        row.property_set("id_artist", *artist.native_handle());
     }
   ::efl::eina::value genre(tag.genre);
   row.property_set("genre", *genre.native_handle());
   ::efl::eina::value year(tag.year);
   row.property_set("year", *year.native_handle());
   return false;
}

bool
audiolistmodel::album_row_inserted(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void *info)
{
   std::cout << "New album values have been set: " << tag.album << std::endl;
   connection->disconnect();

   album_map.insert(std::make_pair(tag.album, row));

   processing_tag = check_track(tag);
   process_pending_tags();
   return false;
}

bool
audiolistmodel::check_track(const tag &tag)
{
   std::cout << "Checking track: " << tag.file << std::endl;
   if (tag.file.empty())
     return false;

   auto it = track_map.find(tag.file);
   if (end(track_map) != it)
     {
        std::cout << "track found: " << tag.file << std::endl;
        return false;
     }

   std::cout << "track not found, creating track: " << tag.file << std::endl;
   // create track
   auto obj = tracks.child_add();
   esql::model_row row(::eo_ref(obj._eo_ptr()));
   auto connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   *connection = row.callback_load_status_add(std::bind(&audiolistmodel::new_track_row_properties_loaded, this, connection, tag, row, std::placeholders::_3));
   row.properties_load();
   return true;
}

bool
audiolistmodel::new_track_row_properties_loaded(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void *info)
{
   const Emodel_Load &st = *static_cast<Emodel_Load*>(info);

   if (st.status & EMODEL_LOAD_STATUS_ERROR)
     {
        connection->disconnect();
        std::cout << "Error loading new track row properties: " << tag.file << std::endl;
        return false;
     }

   if(!(st.status & EMODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return true;

   connection->disconnect();

   int64_t id_artist = -1;
   auto artist_it = artist_map.find(tag.artist);
   if (end(artist_map) != artist_it)
     {
        std::cout << "Getting artist id for: " << tag.artist << std::endl;
        get_property(artist_it->second, "id", id_artist);
        std::cout << "Artist id=" << id_artist << std::endl;
     }
   int64_t id_album = -1;
   auto album_it = album_map.find(tag.album);
   if (end(album_map) != album_it)
     {
        std::cout << "Getting album id for: " << tag.album << std::endl;
        get_property(album_it->second, "id", id_album);
        std::cout << "Album id=" << id_artist << std::endl;
     }

   std::cout << "New track ready to set values: " << tag.file << std::endl;
   auto new_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   *new_connection = row.callback_properties_changed_add(std::bind(&audiolistmodel::track_row_inserted, this, new_connection, tag, row, std::placeholders::_3));
   ::efl::eina::value file(tag.file);
   row.property_set("file", *file.native_handle());
   if (id_artist != -1)
     {
        ::efl::eina::value artist(id_artist);
        row.property_set("id_artist", *artist.native_handle());
     }
   if (id_album != -1)
     {
        ::efl::eina::value album(id_album);
        row.property_set("id_album", *album.native_handle());
     }
   ::efl::eina::value name(tag.title);
   row.property_set("name", *name.native_handle());
   //row.property_set("track", *track.native_handle());
   return false;
}

bool
audiolistmodel::track_row_inserted(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void *info)
{
   std::cout << "New track values have been set: " << tag.file << std::endl;
   connection->disconnect();

   track_map.insert(std::make_pair(tag.file, row));
   processing_tag = false;
   process_pending_tags();
   return false;
}


} //emc
