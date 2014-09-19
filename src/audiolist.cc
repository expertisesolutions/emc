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

#include "audiolist.hh"

namespace emc {

void
audiolist::_on_key_down(std::string key)
{
}

audiolist::audiolist(const ::elm_layout &_layout, const std::string &_theme)
   : basectrl(_layout, _theme, "audiolist")
{
}

} //emc
