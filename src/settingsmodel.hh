#ifndef _SETTINGS_MODEL_HH
#define _SETTINGS_MODEL_HH

#include "database.hh"

#include <Emodel.h>
#include <Emodel.hh>
#include <Elementary.h>
#include <elm_widget.h>
#include <elm_video.eo.hh>

struct elm_layout;
struct elm_win;

namespace emc {

class settingsmodel
{
   public:
     settingsmodel(::elm_win &win, ::elm_layout &layout);
     ~settingsmodel() {}

     std::string video_rootpath_get();
     std::string audio_rootpath_get();
     bool fullscreen_get();

     void video_rootpath_set(std::string path);
     void audio_rootpath_set(std::string path);
     void fullscreen_set(bool fullscreen);

     /**
      * @brief Updates all media files
      */
     void update_media();

     void group_set(const std::string groupname);
     ::elm_win &win;
     ::elm_layout &layout;
     ::emc::database database;
     ::elm_video player;

   private:
     void on_row_loaded(esql::model_row row);

   private:
     std::unordered_map<std::string, esql::model_row> rows;
};

}

#endif
