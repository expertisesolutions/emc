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
#include <elm_layout.eo.hh>

#include "settingsctrl.hh"

namespace emc {

settingsctrl::settingsctrl(const settingsmodel &_settings, const std::function<void()> &_cb)
   : basectrl(_settings, "settings", _cb)
{
    layout.signal_callback_add(groupname+".videopath.clicked", "*",
       std::bind([this]
          {
            std::cout << "videopath selected cb" << std::endl;
          }
       ));

    layout.signal_callback_add(groupname+".musicpath.clicked", "*",
       std::bind([this]
          {
            std::cout << "musicpath selected cb" << std::endl;
          }
       ));
}

void
settingsctrl::active()
{
   basectrl::active();

   layout.text_set(groupname+"/musicpath", settings.audio_rootpath_get());
   layout.text_set(groupname+"/videopath", settings.video_rootpath_get());
}

} //emc
