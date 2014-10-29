/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include <string>
#include <Evas.h>
#include <Elementary.h>
#include <elm_widget.h>
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"

#include "videolist.hh"

namespace emc {

void
videolist::_on_key_down(std::string key)
{
}

videolist::videolist(const ::elm_layout &_layout, const std::string &_theme, settingsmodel &_settings)
   : basectrl(_layout, _theme, "videolist"),
        settings(_settings),
        list(efl::eo::parent = layout),
        view(nullptr),
        selected(nullptr)
{
   layout.signal_callback_add(groupname+".selected.play", "*",
      std::bind([this]{
          Eina_Value value;
          char *filename = NULL;
          selected.property_get("filename", &value);
          if (eina_value_get(&value, &filename) == EINA_FALSE || filename == NULL)
            {
               std::cout << "ERROR: in video list get filename from model or eina_value" << std::endl;
               return;
            }
          std::string path(filename);
          std::cout << "video list open file: " << path << std::endl;

/*          int is_dir = 0;
          selected.property_get("is_dir", &value);
          eina_value_get(&value, &is_dir);
          if (is_dir == EINA_TRUE)
            {
               eio::model model(path);
               model.load();
               eina_value_flush(&value);
               view.model_set(model);
               return;
            }
*/
          eina_value_flush(&value);
          //videoplay.active();
        }
      ));
}

void
videolist::active()
{
   std::cout << "Video list Active" << std::endl;
   basectrl::active();
   eio::model model;
   model.path_set(settings.video_rootpath_get());
   efreet_mime_init();
   model.children_filter_set(
     std::bind([this](const Eina_File_Direct_Info *info)
       {
          if (info->path[info->name_start] == '.' )
            return false;

          //if (info->type == EINA_FILE_DIR)
          //  return true;

          const char *mime = efreet_mime_type_get(info->path);
          if (mime && std::string(mime).compare(0, 5, "video") == 0)
            {
               std::cout << "file:" << info->path << " type:"<< mime << std::endl;
               return true;
            }

          return false;
       }
     , std::placeholders::_2));
//   model.callback_load_status_add(std::bind([this]{std::cout << "video load status change" << std::endl;}));
//   model.callback_children_count_changed_add(std::bind([this]{std::cout << "video children count change" << std::endl;}));
   view = ::elm_view_list(list, ELM_GENLIST_ITEM_NONE, "double_label");
   view.model_set(model);
   view.property_connect("filename", "elm.text");
   view.property_connect("path", "elm.text.sub");

   view.callback_model_selected_add(std::bind([this](void *eo)
       { selected = eio::model(static_cast<Eo *>(eo)); }
                , std::placeholders::_3));

   model.load();
   layout.content_set(groupname+"/list", list);
   list.show();
   eo_unref(list._eo_ptr());
   eo_unref(list._eo_ptr());
}

void
videolist::deactive()
{
   std::cout << "Video List deactive" << std::endl;
   efreet_mime_shutdown();
   layout.content_unset(groupname+"/list");
   selected = eio::model(nullptr);
   view._reset();
   basectrl::deactive();
   list.hide();
}

} //emc
