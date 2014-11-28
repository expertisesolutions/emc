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
   : basectrl(_settings, "settings", _cb),
   m_fentry(efl::eo::parent = layout),
   v_fentry(efl::eo::parent = layout)
{

}

void
settingsctrl::active()
{
   basectrl::active();

   layout.content_set(groupname+"/swallow/musicpath", m_fentry);
   m_fentry.folder_only_set(true);
   m_fentry.path_set(settings.audio_rootpath_get());

   layout.content_set(groupname+"/swallow/videopath", v_fentry);
   v_fentry.folder_only_set(true);
   v_fentry.path_set(settings.video_rootpath_get());

   m_fentry.visibility_set(true);
   v_fentry.visibility_set(true);
}

void
settingsctrl::deactive()
{

   layout.content_unset(groupname+"/swallow/musicpath");
   layout.content_unset(groupname+"/swallow/videopath");

   m_fentry.visibility_set(false);
   v_fentry.visibility_set(false);

   basectrl::deactive();
}

} //emc
