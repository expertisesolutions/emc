#ifndef _AUDIOLIST_MODEL_HH
#define _AUDIOLIST_MODEL_HH

#include "database.hh"
#include "database_map.hh"
#include "file_scanner.hh"
#include "tag_pool.hh"
#include "tag_reader.hh"

#include <Esql_Model.hh>

#include <cstdint>
#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

namespace emc {

class tag;
class tag_processor;

class audiolistmodel
{
   std::string video_dir;
   std::string audio_dir;
   ::emc::database &database;
   ::emc::database_map database_map;
   ::emc::tag_reader tag_reader;
   ::emc::file_scanner scanner;
   ::emc::tag_pool tag_pool;
   std::queue<tag> pending_tags;

   std::queue<std::unique_ptr<tag_processor>> processing_tags;

   void on_database_loaded(bool error);
   void on_rows_mapped();

   void tag_read_cb(const tag &tag);
   void media_file_add_cb(const tag &tag);
   void populate_maps();
   void populate_map(esql::model_table &table, const std::string &key_field, std::unordered_map<std::string, esql::model_row> &map);
   void process_pending_tags();
   void process_tag(const tag &tag);

   bool is_processing_tags() const;
   bool process();
   void next_processor(const tag &tag);

   int64_t artist_id_get(const std::string &artist_name) const;
   int64_t album_id_get(const std::string &album_name) const;
   int64_t track_id_get(const std::string &file_name) const;


   public:
     audiolistmodel(::emc::database &database);
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
