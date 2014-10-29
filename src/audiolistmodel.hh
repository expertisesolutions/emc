#ifndef _AUDIOLIST_MODEL_HH
#define _AUDIOLIST_MODEL_HH

#include <iostream>
#include "Emodel.h"
#include "Emodel.hh"

extern "C"
{
#include <Esskyuehl.h>
#include "Esql_Model.h"
}
#include "Esql_Model.eo.hh"


namespace emc {

class audiolistmodel
{
   std::string video_dir;
   std::string audio_dir;
   esql::model database;
   esql::model_table artists;
   esql::model_table albums;
   esql::model_table tracks;
   std::function<void()> loaded;
   int is_load;

   bool load_tables();

   public:
     audiolistmodel();
     ~audiolistmodel() {}
     esql::model_table& artists_get();
     esql::model_table& albums_get();
     esql::model_table& tracks_get();
     esql::model_table& artist_albums_get(esql::model_row& artist);
     esql::model_table& artist_tracks_get(esql::model_row& artist);
     esql::model_table& album_tracks_get(esql::model_row& album);
};

} //emc
#endif
