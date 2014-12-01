#ifndef _AUDIOLIST_MODEL_HH
#define _AUDIOLIST_MODEL_HH

#include <iostream>
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

   bool init(void * info);
   bool db_table_created(void * info);
   bool load_tables();

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
