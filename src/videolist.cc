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
#include "settingsmodel.hh"

namespace emc {

void
videolist::_on_key_down(std::string key)
{
}

videolist::videolist(const ::elm_layout &_layout, const std::string &_theme, const std::string &_videopath)
   : basectrl(_layout, _theme, "videolist"),
        videopath(_videopath),
        list(efl::eo::parent = layout),
        view(nullptr)
{
   layout.signal_callback_add(groupname+".selected.play", "*",
      std::bind([this]{
          //videoplay.active();
          std::cout << "video play file:" << currentfile << std::endl;
        }
      ));
}

void
videolist::active()
{
   std::cout << "Video list Active" << std::endl;
   basectrl::active();
   eio::model model(videopath);
   model.load();
   view = ::elm_view_list(list, ELM_GENLIST_ITEM_NONE, "double_label");
   view.model_set(model);
   view.property_connect("filename", "elm.text");

   view.callback_model_selected_add(std::bind([this]
       (void *eo)
       {
          Eina_Value value;
//          eio::model selec(static_cast<Eo *>(eo));
//          select.property_get("filename", &value);
          std::cout << "model selected file:" << std::endl;
       }, std::placeholders::_3));

   layout.content_set(groupname+"/list", list);
   list.show();
}

void
videolist::deactive()
{
   std::cout << "Video List deactive" << std::endl;
   layout.content_unset(groupname+"/list");

   view._reset();
   basectrl::deactive();
   list.hide();
}

} //emc
