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

#include "Efreet.h"

#include "settingsmodel.hh"
#include "database_schema.hh"
#include "emodel_helpers.hh"

#define THEME_PATH "../themes/default"
#define WIDTH 1280
#define HEIGHT 720

namespace emc {

settingsmodel::settingsmodel(::elm_win &_win, ::elm_layout &_layout)
   : win(_win)
   , layout(_layout)
   , player(efl::eo::parent = layout)
{
   layout.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(layout);
   win.size_set(WIDTH, HEIGHT);
   win.visibility_set(true);
   layout.visibility_set(true);

   elm_theme_overlay_add(NULL, THEME_PATH"/theme_overlay.edj");
   efreet_init();

   database.load_callback_add([this](bool err) {
      if (!err) {
         auto &table = database.settings_get();
         auto _rows = emc::emodel_helpers::children_get<esql::model_row>(table);
         for (auto &row : _rows) {
            emc::emodel_helpers::async_load(row, std::bind(&settingsmodel::on_row_loaded, this, row));
         }
      }
   });
}

void
settingsmodel::on_row_loaded(esql::model_row row)
{
   std::string property;
   if (emc::emodel_helpers::property_get(row, "key", property))
   this->rows.insert(std::make_pair(property, row));
}

std::string
settingsmodel::video_rootpath_get()
{
   std::string path(efreet_videos_dir_get());
      auto it = rows.find("video_path");
      if (end(rows) != it) {
         //std::cout << table.name_get() << std::endl;
         emodel_helpers::property_get(it->second, "value", path);
      }
   return path;
}

std::string
settingsmodel::audio_rootpath_get()
{
   std::string path(efreet_music_dir_get());
      auto it = rows.find("music_path");
      if (end(rows) != it) {
         //std::cout << table.name_get() << std::endl;
         emodel_helpers::property_get(it->second, "value", path);
      }
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
settingsmodel::video_rootpath_set(std::string path)
{
   auto it = rows.find("video_path");
   if (end(rows) != it) {
      emodel_helpers::property_set(it->second, "value", path);
   } else {
      auto &table = database.settings_get();
      database.async_create_row(table, [this, path](bool err, esql::model_row row)
          {
              std::string value("video_path");
              emodel_helpers::property_set(row, "key", value);
              emodel_helpers::property_set(row, "value", path);
              rows.insert(std::make_pair("video_path", row));
          });
   }
}

void
settingsmodel::audio_rootpath_set(std::string path)
{
   auto it = rows.find("music_path");
   if (end(rows) != it) {
      emodel_helpers::property_set(it->second, "value", path);
   } else {
      auto &table = database.settings_get();
      database.async_create_row(table, [this, path](bool err, esql::model_row row)
          {
              std::string value("music_path");
              emodel_helpers::property_set(row, "key", value);
              emodel_helpers::property_set(row, "value", path);
              rows.insert(std::make_pair("music_path", row));
          });
   }
}

void
settingsmodel::group_set(const std::string groupname)
{
   std::string theme_dir(THEME_PATH);
   layout.file_set(theme_dir + "/default.edj", groupname);
}

} //emc
