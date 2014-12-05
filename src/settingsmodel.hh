#ifndef _SETTINGS_MODEL_HH
#define _SETTINGS_MODEL_HH

#include <iostream>
#include "Emodel.h"
#include "Emodel.hh"
#include <elm_layout.eo.hh>
#include <elm_win.eo.hh>
#include <elm_video.eo.hh>

namespace emc {

class settingsmodel
{
   std::string video_dir;
   std::string audio_dir;
   std::string theme_dir;

   public:
     settingsmodel(::elm_win &win, ::elm_layout &layout);
     ~settingsmodel() {}

     std::string video_rootpath_get();
     std::string audio_rootpath_get();
     bool fullscreen_get();

     void video_rootpath_set(std::string path);
     void audio_rootpath_set(std::string path);
     void fullscreen_set(bool fullscreen);

//     ::elm_video player_get();
     void group_set(const std::string groupname);
     ::elm_win &win;
     ::elm_layout &layout;
     ::elm_video player;
};

} //emc
#endif
