#ifndef _AUDIOLIST_MODEL_HH
#define _AUDIOLIST_MODEL_HH

#include <cstdint>
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
class tag_processor;

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
   std::unordered_map<std::string, esql::model_row> track_map;
   std::unordered_map<std::string, esql::model_row> artist_map;
   std::unordered_map<std::string, esql::model_row> album_map;
   std::queue<std::unique_ptr<tag_processor>> processing_tags;

   bool init(void * info);
   bool db_table_created(void * info);
   bool load_tables();
   void media_file_add_cb(const tag &tag);
   void populate_maps();
   void populate_map(esql::model_table &table, const std::string &key_field, std::unordered_map<std::string, esql::model_row> &map);
   void process_pending_tags();
   void process_tag(const tag &tag);

   bool is_processing_tags() const;
   void next_processor();

   int64_t artist_id_get(const std::string &artist_name) const;
   int64_t album_id_get(const std::string &album_name) const;
   int64_t track_id_get(const std::string &file_name) const;


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
