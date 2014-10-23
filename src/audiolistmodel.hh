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
   void model_get(std::function<void(efl::eo::base&)> func, esql::model_table& model);
   int is_load;

   bool load_tables();

   public:
     audiolistmodel();
     ~audiolistmodel() {}
     void artists_get(std::function<void(efl::eo::base&)> func);
     void albums_get(std::function<void(efl::eo::base&)> func);
     void tracks_get(std::function<void(efl::eo::base&)> func);
     void artist_albums_get(efl::eo::base &artist, std::function<void(efl::eo::base&)> func);
     void artist_tracks_get(efl::eo::base &artist, std::function<void(efl::eo::base&)> func);
     void albums_tracks_get(efl::eo::base &album, std::function<void(efl::eo::base&)> func);
};

} //emc
#endif
