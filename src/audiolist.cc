/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */
#include "audiolist.hh"

#include <Ecore.hh>
#include <Evas.h>
#include <Evas.hh>
#include <Elementary.h>
#include <elm_widget.h>
#include <elm_interface_atspi_accessible.h>
#include <elm_interface_atspi_accessible.eo.h>
#include <elm_interface_atspi_widget_action.h>
#include <elm_interface_atspi_widget_action.eo.h>
#include <elm_layout.eo.hh>
#include <Evas.h>

#include "emodel_helpers.hh"
#include "logger.hh"
#include "audiolist.hh"
#include <Ecore.hh>
#include <thread>
#include <iomanip>

#include <string.h>

namespace emc {

void
audiolist::_on_key_down(std::string key)
{
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
playback_started_cb(void *data, Evas_Object *obj, void *event_info)
{
   audiolist *t = static_cast<audiolist*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }

   efl::ecore::main_loop_thread_safe_call_async(std::bind(&audiolist::playback_update, t));
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

audiolist::audiolist(::emc::database &database, settingsmodel &_settings, const std::function<void()> &_cb)
   : basectrl(_settings, "audiolist", _cb),
        list(efl::eo::parent = layout),
        progslider(efl::eo::parent = layout),
        view(nullptr),
        row_selected(nullptr),
        model(database)
        playlist(settings)
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
            emodel em(eo_ref(playlist._eo_ptr()));
            view.model_set(em);
          }
       ));

    layout.signal_callback_add("audiolist.playlist.playpause", "*",
       std::bind([this]
          {
            std::cout << "playpause selected cb" << std::endl;
            if (settings.player.is_playing_get()) {
              playlist.pause();
              layout.signal_emit("audiolist.playlist.paused", "");
            } else
              playlist.play();
          }
       ));

    layout.signal_callback_add("audiolist.playlist.next", "*", std::bind([this]{playlist.play_next();}));
    layout.signal_callback_add("audiolist.playlist.prev", "*", std::bind([this]{playlist.play_prev();}));

    layout.signal_callback_add("audiolist.playlist.repeat", "*",
       std::bind([this]
          {
            std::cout << "repeat selected cb" << std::endl;
          }
       ));
    layout.signal_callback_add("audiolist.playlist.random", "*",
       std::bind([this]
          {
            std::cout << "random selected cb" << std::endl;
          }
       ));

    progslider.callback_changed_add(
      std::bind([this]
          {
              // Update video position. Progress slider is updated as side effect.
              settings.player.play_position_set(progslider.value_get());
          }
      ));
}

void
audiolist::player_fame_decode_cb()
{
    std::ostringstream label_total, label_pos;
    int h, m, s;

    double pos = settings.player.play_position_get();
    double len = settings.player.play_length_get();

    h = pos / 3600;
    m = pos / 60 - (h * 60);
    s = pos - (h * 3600) - (m * 60);
    label_pos.fill('0');
    label_pos << std::setw(2) << m << ":" << std::setw(2) << s;

    h = len / 3600;
    m = len / 60 - (h * 60);
    s = len - (h * 3600) - (m * 60);
    label_total.fill('0');
    label_total << std::setw(2) << m << ":" << std::setw(2) << s;

    layout.text_set("music_temp_restante", label_pos.str());
    layout.text_set("music_temp_total", label_total.str());

    progslider.min_max_set(0, len);
    progslider.value_set(pos);
}

void
audiolist::playback_update()
{
    if (!settings.player.is_playing_get())
      return;

    layout.signal_emit("audiolist.playlist.playing", "");
    std::string name;
    auto track = playlist.curr();
    if (emc::emodel_helpers::property_get(track, "name", name)) {
       layout.text_set("audiolist/track", name);
    }

    Eina_Value value = {};
    if (track.property_get("artwork", &value)) {
       Eina_Value_Blob blob = {};
       eina_value_get(&value, &blob);
       if (blob.memory && blob.size > 0) {
          evas::image img(efl::eo::parent = layout);
          evas_object_image_memfile_set(img._eo_ptr(), const_cast<void*>(blob.memory), blob.size, nullptr, nullptr);
          img.filled_set(true);
          img.visibility_set(true);
          layout.content_set("audiolist/artwork", img);
       } else {
          auto c = layout.content_get("audiolist/artwork");
          c.visibility_set(false);
          layout.content_unset("audiolist/artwork");
       }
    }
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
       emodel m(nullptr);
       view.model_get(&m);
       auto l = emc::emodel_helpers::children_get<esql::model_row>(m);
       m._release();
       playlist.list_set(l);
       playlist.play(row_selected);
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
   progslider.visibility_set(true);
   view = ::elm_view_list(view.elm_view_list_constructor(list, ELM_GENLIST_ITEM_NONE, "default"));

   view.callback_model_selected_add(std::bind([this](void *eo)
      {
         row_selected = esql::model_row(eo_ref(static_cast<Eo *>(eo)));
      }, std::placeholders::_3));

   layout.content_set(groupname+"/list", list);
   layout.content_set(groupname+"/progressbar", progslider);

   list.show();
   progslider.show();
   playback_update();

   evas::object emotion = settings.player.emotion_get();
   evas_object_smart_callback_add(emotion._eo_ptr(), "frame_decode", frame_decode_cb, this); //FIXME
   evas_object_smart_callback_add(emotion._eo_ptr(), "playback_started", playback_started_cb, this); //FIXME
   evas_object_smart_callback_add(list._eo_ptr(), "activated", activated_cb, this); //FIXME

   view.model_set(model.artists_get());
   view.property_connect("name", "elm.text");
   view.property_connect("artwork", "elm.swallon.icon");
}

void
audiolist::deactive()
{
   evas::object emotion = settings.player.emotion_get();
   evas_object_smart_callback_del(emotion._eo_ptr(), "frame_decode", frame_decode_cb); //FIXME
   evas_object_smart_callback_del(emotion._eo_ptr(), "playback_started", playback_started_cb); //FIXME
   evas_object_smart_callback_del(list._eo_ptr(), "activated", activated_cb); //FIXME

   layout.content_unset(groupname+"/list");
   layout.content_unset(groupname+"/progressbar");
   view._reset();
   basectrl::deactive();
   list.hide();
   progslider.hide();
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
