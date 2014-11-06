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
#include <Ecore.hh>
#include <thread>

namespace emc {

void
audiolist::_on_key_down(std::string key)
{
}
 //FIXME
static void
playback_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
   audiolist *t = static_cast<audiolist*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }
   efl::ecore::main_loop_thread_safe_call_async(std::bind(&audiolist::player_playback_finished_cb, t));
}
 //FIXME
static void
frame_decode_cb(void *data, Evas_Object *obj, void *event_info)
{
   audiolist *t = static_cast<audiolist*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }

   efl::ecore::main_loop_thread_safe_call_async(std::bind(&audiolist::player_fame_decode_cb, t));
}
 //FIXME
static void
activated_cb(void *data, Evas_Object *obj, void *event_info)
{
   audiolist *t = static_cast<audiolist*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }

   efl::ecore::main_loop_thread_safe_call_async(std::bind(&audiolist::list_activated_cb, t));

}

audiolist::audiolist(const ::elm_layout &_layout, const std::string &_theme, settingsmodel &_settings)
   : basectrl(_layout, _theme, "audiolist"),
        settings(_settings),
        list(efl::eo::parent = layout),
        player(efl::eo::parent = layout),
        view(nullptr),
        row_selected(nullptr),
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

    evas::object emotion = player.emotion_get();
    evas_object_smart_callback_add(emotion._eo_ptr(), "playback_finished", playback_finished_cb, this); //FIXME
    evas_object_smart_callback_add(emotion._eo_ptr(), "frame_decode", frame_decode_cb, this); //FIXME

    evas_object_smart_callback_add(list._eo_ptr(), "activated", activated_cb, this); //FIXME
}

void
audiolist::player_playback_finished_cb()
{
    Elm_Object_Item* i = list.selected_item_get();
    if (!i) return;

    if (player.is_playing_get())
            std::cout << "PORRA" << std::endl;
    Elm_Object_Item* next = elm_genlist_item_next_get(i);
    if (!next) return;

    elm_genlist_item_selected_set(next, EINA_TRUE);
}

void
audiolist::player_fame_decode_cb()
{
    std::ostringstream label_total, label_pos;
    int h, m, s;

    double pos = player.play_position_get();
    double len = player.play_length_get();

    h = pos / 3600;
    m = pos / 60 - (h * 60);
    s = pos - (h * 3600) - (m * 60);

    label_pos << m << ":" << s;

    h = len / 3600;
    m = len / 60 - (h * 60);
    s = len - (h * 3600) - (m * 60);

    label_total << m << ":" << s;

    layout.text_set("music_temp_restante", label_pos.str());
    layout.text_set("music_temp_total", label_total.str());
}

void
audiolist::list_activated_cb()
{
    if (row_selected._eo_ptr() == NULL)
      return;

    std::string tablename = row_selected.tablename_get();
    std::cout << tablename << std::endl;
    if (tablename == "artists")
       view.model_set(model.artist_albums_get(row_selected));
    else if (tablename == "albums")
       view.model_set(model.album_tracks_get(row_selected));
    else if (tablename == "tracks") {
       Eina_Value v;
       row_selected.property_get("file", &v);
       char *path = eina_value_to_string(&v);
       if (path) {
         std::cout << std::this_thread::get_id() << std::endl;
         player.file_set(path, "");
         player.play();
       }
       free(path);
    }
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
//   elm_theme_overlay_add(NULL, "../themes/default/theme_overlay.edj");
   view = ::elm_view_list(list, ELM_GENLIST_ITEM_NONE, "default");

   view.callback_model_selected_add(std::bind([this](void *eo)
      {
         row_selected = esql::model_row(static_cast<Eo *>(eo));
      }, std::placeholders::_3));

   layout.content_set(groupname+"/list", list);
   list.show();
   eo_unref(list._eo_ptr()); //XXX
   eo_unref(list._eo_ptr()); //XXX

   view.model_set(model.artists_get());
   view.property_connect("name", "elm.text");
//   view.property_connect("name", "elm.text.sub");
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
