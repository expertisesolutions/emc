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
        view(nullptr)
{
}

void
videolist::active()
{
   std::cout << "Video list Active" << std::endl;
   basectrl::active();
   eio::model model(videopath);
   model.load();
   ::elm_genlist genlist(efl::eo::parent = layout);
   view = ::elm_view_list(genlist, ELM_GENLIST_ITEM_NONE, "double_label");

   view.model_set(model);
   view.property_connect("filename", "elm.text");
   layout.content_set(groupname+"/list", genlist);
   genlist.visibility_set(true);
}

void
videolist::deactive()
{
   std::cout << "Video List deactive" << std::endl;
   layout.content_unset(groupname+"/list");

   view._reset();
   basectrl::deactive();
}

} //emc
