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

//Constructor
basectrl::basectrl(const ::elm_layout &_layout, const std::string &_theme, const std::string &_groupname) :
        groupname(_groupname),
        layout(_layout),
        theme(_theme),
        deactive_cb()
{
}

void
basectrl::active()
{
   layout.file_set(theme, groupname);
}

void
basectrl::deactive()
{
   deactive_cb();
}

bool
basectrl::theme_set(const std::string &_theme)
{
   theme = _theme;
   return layout.file_set(theme, groupname);
}

bool
basectrl::deactive_cb_set(std::function<void()> _deactive_cb)
{
   deactive_cb = _deactive_cb;
}

} //emc
