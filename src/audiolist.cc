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
        layout.signal_callback_add("audiolist.selected.artists", "*",
           std::bind([this]
              {
                std::cout << "artists selected cb" << std::endl;
                view.model_set(model.artists_get());
              }
           ));

        layout.signal_callback_add("audiolist.selected.albums", "*",
           std::bind([this]
              {
                std::cout << "albums selected cb" << std::endl;
                view.model_set(model.albums_get());
              }
           ));

        layout.signal_callback_add("audiolist.selected.playlist", "*",
           std::bind([this]
              {
                std::cout << "playlist selected cb" << std::endl;
              }
           ));


}

void
audiolist::active()
{
   Eina_Accessor *accessor = NULL;
   Eina_Value value_prop;
   Eo *child;
   unsigned int i = 0;

   basectrl::active();

   list.visibility_set(true);
   view = ::elm_view_list(list, ELM_GENLIST_ITEM_NONE, "default");
   view.callback_model_selected_add(std::bind([this](void *eo)
      {
         esql::model_row m(static_cast<Eo *>(eo));
         std::string tablename = m.tablename_get();
         std::cout << tablename << std::endl;
         if (tablename == "artists")
            view.model_set(model.artist_albums_get(m));
         else if (tablename == "albums")
            view.model_set(model.album_tracks_get(m));
      }, std::placeholders::_3));

   layout.content_set(groupname+"/list", list);
   list.show();
   eo_unref(list._eo_ptr());
   eo_unref(list._eo_ptr());

   view.model_set(model.artists_get());
   view.property_connect("name", "elm.text");
}

void
audiolist::deactive()
{
   layout.content_unset(groupname+"/list");
   view._reset();
   basectrl::deactive();
   list.hide();
}

void
audiolist::artists_show(esql::model_table& model)
{}

void
audiolist::albums_show(esql::model_table& model)
{}

void
audiolist::playlists_show(esql::model_table& model)
{}




} //emc
