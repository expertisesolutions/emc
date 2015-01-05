#ifndef _SETTINGS_MODEL_HH
#define _SETTINGS_MODEL_HH

#include <Emodel.h>
#include <Emodel.hh>
#include <Elementary.h>
#include <elm_widget.h>
#include <elm_video.eo.hh>

#include <functional>

struct elm_layout;
struct elm_win;

namespace emc {

class database;
class database_map;

class settingsmodel
{
   public:
     settingsmodel(::emc::database &database,
                   ::emc::database_map &database_map,
                   ::elm_win &win,
                   ::elm_layout &layout);
     ~settingsmodel();

     std::string video_rootpath_get() const;
     std::string audio_rootpath_get() const;
     bool fullscreen_get() const;

     void video_rootpath_set(const std::string &path);
     void audio_rootpath_set(const std::string &path);
     void fullscreen_set(bool fullscreen);

     void group_set(const std::string groupname);

     // FIX: Remove this GUI pieces from the application domain
     ::elm_win &win;
     ::elm_layout &layout;
     ::elm_video player;

   private:
     ::emc::database &database;
     ::emc::database_map &database_map;
};

}

#endif
