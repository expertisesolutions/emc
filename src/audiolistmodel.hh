 #ifndef _AUDIOLIST_MODEL_HH
#define _AUDIOLIST_MODEL_HH

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include <eo_event.hh>
#include <Emodel.h>
#include <Emodel.hh>

extern "C"
{
#include <Esskyuehl.h>
#include <Esql_Model.h>
}
#include <Esql_Model.hh>

#include "file_scanner.hh"

namespace emc {

class tag;

class audiolistmodel
{
   std::string video_dir;
   std::string audio_dir;
   esql::model database;
   esql::model_table artists;
   esql::model_table albums;
   esql::model_table tracks;

   file_scanner scanner;
   ::efl::eo::signal_connection init_connection;
   ::efl::eo::signal_connection db_table_created_connection;

   std::queue<tag> pending_tags;

   bool maps_ready;
   bool processing_tag;
   std::unordered_map<std::string, esql::model_row> track_map;
   std::unordered_map<std::string, esql::model_row> artist_map;
   std::unordered_map<std::string, esql::model_row> album_map;

   bool init(void * info);
   bool db_table_created(void * info);
   bool load_tables();
   void media_file_add_cb(const tag &tag);
   void populate_maps();
   void populate_map(const esql::model_table &table, const std::string &key, std::unordered_map<std::string, esql::model_row> &map);
   void process_pending_tags();
   bool process_tag(const tag &tag);

   bool check_artist(const tag &tag);
   bool new_artist_row_properties_loaded(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void * info);
   bool artist_row_inserted(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void * info);

   bool check_album(const tag &tag);
   bool new_album_row_properties_loaded(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void * info);
   bool album_row_inserted(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void * info);

   bool check_track(const tag &tag);
   bool new_track_row_properties_loaded(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void * info);
   bool track_row_inserted(std::shared_ptr<::efl::eo::signal_connection> connection, tag tag, esql::model_row row, void * info);

   public:
     audiolistmodel();
     ~audiolistmodel();
     esql::model_table& artists_get();
     esql::model_table& albums_get();
     esql::model_table& tracks_get();
     esql::model_table& artist_albums_get(esql::model_row& artist);
     esql::model_table& artist_tracks_get(esql::model_row& artist);
     esql::model_table& album_tracks_get(esql::model_row& album);
};

} //emc

#endif
