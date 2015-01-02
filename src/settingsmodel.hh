#ifndef _SETTINGS_MODEL_HH
#define _SETTINGS_MODEL_HH

#include <Emodel.h>
#include <Emodel.hh>
#include <Elementary.h>
#include <elm_widget.h>
#include <elm_video.eo.hh>

struct elm_layout;
struct elm_win;

namespace emc {

class database;
class database_map;

class settingsmodel
{
   public:
     settingsmodel(::emc::database &database, ::emc::database_map &database_map, ::elm_win &win, ::elm_layout &layout);
     ~settingsmodel();

     std::string video_rootpath_get();
     std::string audio_rootpath_get();
     bool fullscreen_get();

     void video_rootpath_set(const std::string &path);
     void audio_rootpath_set(const std::string &path);
     void fullscreen_set(bool fullscreen);

     /**
      * @brief Updates all media files
      */
     void update_media();

     void group_set(const std::string groupname);
     ::elm_win &win;
     ::elm_layout &layout;
     ::elm_video player;

   private:
     ::emc::database &database;
     ::emc::database_map &database_map;

};

}

#endif
