/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include <Elementary.h>
#include <elm_widget.h>
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"
#include <elm_layout.eo.hh>


#include "settingsmodel.hh"

#define VIDEODIR "/Videos"
#define AUDIODIR "/Music"
#define PICTUREDIR "/Pictures"

#define THEME_PATH "../themes/default"

#define WIDTH 1280
#define HEIGHT 720

namespace emc {

settingsmodel::settingsmodel(::elm_win &_win, ::elm_layout &_layout)
   : win(_win),
     layout(_layout),
     player(efl::eo::parent = layout),
     theme_dir(THEME_PATH)
{
   layout.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(layout);
   win.size_set(WIDTH, HEIGHT);
   win.visibility_set(true);
   layout.visibility_set(true);

   std::string homepath(".");
   char *tmp = getenv("HOME");
   std::cout << tmp << std::endl;

   if(tmp) homepath = std::string(tmp);

   video_dir = std::string(homepath + VIDEODIR);
   audio_dir = std::string(homepath + AUDIODIR);

   elm_theme_overlay_add(NULL, THEME_PATH"/theme_overlay.edj");
}

std::string
settingsmodel::video_rootpath_get()
{
   std::cout << "settings videopath get " << video_dir << std::endl;
   return video_dir;
}

std::string
settingsmodel::audio_rootpath_get()
{
   return audio_dir;
}

void
settingsmodel::video_rootpath_set(std::string path)
{
   video_dir = path;
}

void
settingsmodel::audio_rootpath_set(std::string path)
{
   audio_dir = path;
}


void
settingsmodel::group_set(const std::string groupname)
{
   layout.file_set(theme_dir + "/default.edj", groupname);
}

} //emc
