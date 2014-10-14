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

audiolist::audiolist(const ::elm_layout &_layout, const std::string &_theme, settingsmodel &_settings)
   : basectrl(_layout, _theme, "audiolist"),
        settings(_settings),
        list(efl::eo::parent = layout),
        view(nullptr),
        modelDB(nullptr)
{
   esql_init();
   modelDB = esql::model("./emc.db", "emc_user", "pass123", "audioList");
   modelDB.load();
}

void
audiolist::active()
{
   Eina_Accessor *accessor;

   std::cout << "Audio list Active" << std::endl;
   basectrl::active();
   modelDB.children_slice_get(0, 0, &accessor);
   view = ::elm_view_list(list, ELM_GENLIST_ITEM_NONE, "default");
}

void
audiolist::deactive()
{
   basectrl::deactive();
}
} //emc
