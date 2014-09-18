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
basectrl::basectrl(const ::elm_win &_win, const ::elm_layout &_layout, const std::string &theme, const std::string &_groupname) :
        groupname(_groupname),
        layout(_layout),
        win(_win),
        deactive_cb()
{
   theme_set(theme);
}

bool
basectrl::active()
{
   layout.visibility_set(true);
}

bool
basectrl::deactive()
{
   layout.visibility_set(false);
   deactive_cb();
}

bool
basectrl::theme_set(const std::string &theme)
{
   layout.file_set(theme, groupname);
}

bool
basectrl::deactive_cb_set(std::function<void()> &_deactive_cb)
{
   deactive_cb = _deactive_cb;
}


} //emc
