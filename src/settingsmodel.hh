#ifndef _SETTINGS_MODEL_HH
#define _SETTINGS_MODEL_HH

#include <iostream>

namespace emc {

//TODO: SqlModel
class settingsmodel
{
   settingsmodel();
   std::string video_dir;
   std::string audio_dir;

   public:
     static settingsmodel& get();
     ~settingsmodel() {}
     std::string videopath_get();
     std::string audiopath_get();
};

} //emc
#endif
