#ifndef _AUDIOLIST_MODEL_HH
#define _AUDIOLIST_MODEL_HH

#include "tagging_service.hh"

#include <Esql_Model.hh>

namespace emc {

class database;

class audiolistmodel
{
   std::string video_dir;
   std::string audio_dir;
   ::emc::database &database;
   ::emc::tagging_service tagging_service;

   void on_database_loaded(bool error);

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
