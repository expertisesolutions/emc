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

#include "basectrl.hh"

namespace emc {

basectrl::basectrl(const settingsmodel &_settings, const std::string &_groupname, const std::function<void()> &_cb) :
        groupname(_groupname),
        settings(_settings),
        deactive_cb(_cb),
        layout(settings.layout),
        key_con(nullptr)
{
   layout.signal_callback_add(groupname+".selected.back", "*",
      std::bind([this]
        {
          deactive();
        }
      ));
}

void
basectrl::active()
{
   key_con = settings.win.callback_key_down_add(std::bind([this](void *einfo)
         {
           on_key_down(static_cast<Evas_Event_Key_Down *>(einfo)->key);
         }, std::placeholders::_3));

   evas_object_focus_set(settings.win._eo_ptr(), EINA_TRUE);
   settings.group_set(groupname);
}

void
basectrl::deactive()
{
   key_con.disconnect();
   deactive_cb();
}

void
basectrl::on_key_down(std::string key)
{
   if (key == "Escape")
     layout.signal_emit(groupname+".select.back", "");
}

void
basectrl::push(basectrl &ctrl)
{
   key_con.disconnect();
   ctrl.active();
}

} //emc
