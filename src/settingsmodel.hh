#ifndef _SETTINGS_MODEL_HH
#define _SETTINGS_MODEL_HH

#include <iostream>
#include "Emodel.h"
#include "Emodel.hh"
#include <elm_layout.eo.hh>
#include <elm_win.eo.hh>

namespace emc {

//TODO: SqlModel
class settingsmodel
{
   std::string video_dir;
   std::string audio_dir;
   std::string theme_dir;

   public:
     settingsmodel(const ::elm_win &win, const ::elm_layout &layout);
     ~settingsmodel() {}
     std::string video_rootpath_get();
     std::string audio_rootpath_get();
     void group_set(const std::string groupname);
     ::elm_win win;
     ::elm_layout layout;
};

} //emc
#endif
