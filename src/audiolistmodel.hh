#ifndef _AUDIOLIST_MODEL_HH
#define _AUDIOLIST_MODEL_HH

#include "database.hh"
#include "database_map.hh"
#include "file_scanner.hh"
#include "tag_pool.hh"
#include "tag_reader.hh"
#include "tag_updater.hh"

#include <Esql_Model.hh>

#include <queue>

namespace emc {

class tag;

class audiolistmodel
{
   std::string video_dir;
   std::string audio_dir;
   ::emc::database &database;
   ::emc::database_map database_map;
   ::emc::tag_reader tag_reader;
   ::emc::file_scanner scanner;
   ::emc::tag_pool tag_pool;
   ::emc::tag_updater updater;
   std::queue<tag> pending_tags;

   void on_database_loaded(bool error);
   void on_rows_mapped();

   void on_tag_read(const tag &tag);
   void media_file_add_cb(const tag &tag);
   void process_pending_tags();

   void on_tag_updated(const tag &tag);

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
