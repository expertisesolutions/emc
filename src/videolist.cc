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
#include <Ecore.hh>
#include <thread>

#include "videolist.hh"

namespace emc {

void
videolist::_on_key_down(std::string key)
{
}

videolist::videolist(settingsmodel &_settings, const std::function<void()> &_cb)
   : basectrl(_settings, "videolist", _cb),
        list(efl::eo::parent = layout),
        view(nullptr),
        selected(nullptr),
        vp(settings, [this]{push_back();})

{
}

 //FIXME
static void
_video_list_activated_cb(void *data, Evas_Object *obj, void *event_info)
{
   videolist *t = static_cast<videolist*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }
   efl::ecore::main_loop_thread_safe_call_async(std::bind(&videolist::list_activated_cb, t));
}

void
videolist::list_activated_cb()
{
   if (selected._eo_ptr() == NULL)
      return;

   layout.content_unset(groupname+"/list");
   list.hide();
   push(vp);
   vp.play(selected);
}

void
videolist::push_back()
{
   basectrl::active();
   layout.content_set(groupname+"/list", list);
   list.show();
}

void
videolist::active()
{
   std::cout << "Video list Active" << std::endl;
   basectrl::active();
   eio::model model;
   model.path_set(settings.video_rootpath_get());
   model.load();
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

   view = ::elm_view_list(view.elm_view_list_constructor(list, ELM_GENLIST_ITEM_NONE, "double_label"));

   view.model_set(model);
   view.property_connect("filename", "elm.text");
   view.property_connect("size", "elm.text.sub");
   view.callback_model_selected_add(std::bind([this](void *eo)
       {
         selected = eio::model(eo_ref(static_cast<Eo *>(eo)));
       }, std::placeholders::_3));

   evas_object_smart_callback_add(list._eo_ptr(), "activated", _video_list_activated_cb, this); //FIXME
   layout.content_set(groupname+"/list", list);
   list.show();
}

void
videolist::deactive()
{
   std::cout << "Video List deactive" << std::endl;
   efreet_mime_shutdown();
   evas_object_smart_callback_del(list._eo_ptr(), "activated", _video_list_activated_cb); //FIXME
   layout.content_unset(groupname+"/list");
   selected._reset();
   view._reset();
   list.hide();
   basectrl::deactive();
}

} //emc
