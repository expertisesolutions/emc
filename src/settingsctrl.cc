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
}

} //emc
