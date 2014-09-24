/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include <Evas.h>
#include <Elementary.h>
#include <elm_widget.h>
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"

#include "videolist.hh"
#include "settingsmodel.hh"
#include <elm_slider.eo.hh>
#include <elm_layout.eo.hh>

namespace emc {

void
videolist::_on_key_down(std::string key)
{
}

videolist::videolist(const ::elm_layout &_layout, const std::string &_theme)
   : basectrl(_layout, _theme, "videolist"),
        genlist(efl::eo::parent = layout)
{
   std::cout << settingsmodel::get().videopath_get() << std::endl;
}

void
videolist::active()
{
   basectrl::active();
   layout.content_set(groupname+"/list", genlist);
}

} //emc
