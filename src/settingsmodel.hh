#ifndef _SETTINGS_MODEL_HH
#define _SETTINGS_MODEL_HH

#include <iostream>
#include "Emodel.h"
#include "Emodel.hh"

namespace emc {

//TODO: SqlModel
class settingsmodel
{
   std::string video_dir;
   std::string audio_dir;

   public:
     settingsmodel();
     ~settingsmodel() {}
     std::string video_rootpath_get();
     std::string audio_rootpath_get();
};

} //emc
#endif
