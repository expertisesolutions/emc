/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */
#include "settingsmodel.hh"

#include "database.hh"
#include "database_map.hh"
#include "database_schema.hh"
#include "emodel_helpers.hh"
#include "logger.hh"

#include <Efreet.h>
#include <Elementary.h>
#include <elm_widget.h>
#include <elm_interface_atspi_accessible.h>
#include <elm_interface_atspi_accessible.eo.h>
#include <elm_interface_atspi_widget_action.h>
#include <elm_interface_atspi_widget_action.eo.h>
#include <elm_layout.eo.hh>
#include <elm_win.eo.hh>


namespace {
   const std::string THEME_PATH = "../themes/default";
   const auto THEME_OVERLAY_PATH = THEME_PATH + "/theme_overlay.edj";
   const auto THEME_DEFAULT_PATH = THEME_PATH + "/default.edj";
   const auto WIDTH = 1280;
   const auto HEIGHT = 720;
   const auto MUSIC_PATH_KEY = "music_path";
   const auto VIDEO_PATH_KEY = "video_path";
}

namespace emc {

settingsmodel::settingsmodel(::emc::database &database, ::emc::database_map &database_map, ::elm_win &_win, ::elm_layout &_layout)
   : database(database)
   , database_map(database_map)
   , win(_win)
   , layout(_layout)
   , player(efl::eo::parent = layout)
{
   layout.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(layout);
   win.size_set(WIDTH, HEIGHT);
   win.visibility_set(true);
   layout.visibility_set(true);

   elm_theme_overlay_add(NULL, THEME_OVERLAY_PATH.c_str());
   efreet_init();
}

settingsmodel::~settingsmodel()
{}

std::string
settingsmodel::video_rootpath_get()
{
   std::string path = database_map.setting_get(VIDEO_PATH_KEY);

   if (path.empty())
     path = efreet_videos_dir_get();

   return path;
}

std::string
settingsmodel::audio_rootpath_get()
{
   std::string path = database_map.setting_get(MUSIC_PATH_KEY);

   if (path.empty())
     path = efreet_music_dir_get();

   return path;
}

bool
settingsmodel::fullscreen_get()
{
   //TODO: get from database
   return win.fullscreen_get();
}

void
settingsmodel::fullscreen_set(bool fullscreen)
{
   //TODO: save in database
   win.fullscreen_set(fullscreen);
}

void
settingsmodel::video_rootpath_set(const std::string &path)
{
   database_map.setting_set(VIDEO_PATH_KEY, path);
}

void
settingsmodel::audio_rootpath_set(const std::string &path)
{
   database_map.setting_set(MUSIC_PATH_KEY, path);
}

void
settingsmodel::group_set(const std::string groupname)
{
   layout.file_set(THEME_DEFAULT_PATH, groupname);
}

void
settingsmodel::update_media()
{
   database.async_reset_media_tables(
     [](bool error)
     {
        if (error)
          {
             ERR << "Error updating media files";
             return;
          }

        // TODO: Restart tagging service
     });
}

}
