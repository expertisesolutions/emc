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
        model()
{
}

void
audiolist::active()
{
   Eina_Accessor *accessor = NULL;
   Eina_Value value_prop;
   Eo *child;
   unsigned int i = 0;

   basectrl::active();

   model.tracks_get([this](efl::eo::base artists)
      {
        std::cout << "artist get" << std::endl;
        list.visibility_set(true);
        view = ::elm_view_list(list, ELM_GENLIST_ITEM_NONE, "default");
        view.model_set(artists);
        view.property_connect("name", "elm.text");
        layout.content_set(groupname+"/artists", list);
        list.show();
      });
}

void
audiolist::deactive()
{
   layout.content_unset(groupname+"/artist");
   view._reset();
   basectrl::deactive();
   list.hide();
}

} //emc
