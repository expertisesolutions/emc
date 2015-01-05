/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */
#include "videoplayer.hh"

#include <Ecore.hh>
#include <Elementary.h>
#include <elm_widget.h>
#include <elm_interface_atspi_accessible.h>
#include <elm_interface_atspi_accessible.eo.h>
#include <elm_interface_atspi_widget_action.h>
#include <elm_interface_atspi_widget_action.eo.h>
#include <elm_layout.eo.hh>
#include <elm_win.eo.hh>
#include <Evas.h>

#include <iomanip>

namespace emc {

 //FIXME
static void
_video_playback_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
   videoplayer *t = static_cast<videoplayer*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }

   efl::ecore::main_loop_thread_safe_call_async(std::bind(&videoplayer::deactive, t));
}

 //FIXME
static void
_video_open_done_cb(void *data, Evas_Object *obj, void *event_info)
{
   videoplayer *t = static_cast<videoplayer*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }

   efl::ecore::main_loop_thread_safe_call_async(std::bind(&videoplayer::opened_done_cb, t));
}

 //FIXME
static void
_video_frame_decode_cb(void *data, Evas_Object *obj, void *event_info)
{
   videoplayer *t = static_cast<videoplayer*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }

   efl::ecore::main_loop_thread_safe_call_async(std::bind(&videoplayer::player_fame_decode_cb, t));
}

videoplayer::videoplayer(settingsmodel &settings, const std::function<void()> &deactive_cb)
   : basectrl(settings, "videoplayer", deactive_cb),
        player(settings.player),
        progslider(efl::eo::parent = layout)
{
    layout.signal_callback_add("videoplayer.selected.playpause", "*",
       std::bind([this]
          {
             if (player.is_playing_get()) {
               player.pause();
               layout.signal_emit("videoplayer.video.paused", "");
             } else {
               player.play();
               layout.signal_emit("videoplayer.video.playing", "");
             }
          }
       ));

   progslider.callback_changed_add
      (std::bind([this]
          {
              // Update video position. Progress slider is updated as side effect.
              player.play_position_set(progslider.value_get());
          }
      ));
}

void
videoplayer::play(emodel model)
{
   Eina_Value v = {};

   if (player.is_playing_get())
     player.stop();

   layout.content_set(groupname+"/video", player);
   player.show();

   if (model.property_get("path", &v) == EMODEL_LOAD_STATUS_ERROR) {
      std::cout << "ERROR GET FILENAME MODEL PROPERTY" << std::endl;
      return;
   }

   if (nullptr == v.type) {
      std::cout << "ERROR TYPE EINA VALUE IS NULL" << std::endl;
      return;
   }

   evas::object emotion = player.emotion_get();
   char *path = eina_value_to_string(&v);
   if (path) {
      evas_object_smart_callback_add(emotion._eo_ptr(), "open_done", _video_open_done_cb, this); //FIXME
      player.file_set(path, "");
      free(path);
   }

   model.property_get("filename", &v);
   char *name = eina_value_to_string(&v);
   if (name) {
      layout.text_set(groupname+"/filename", name);
      free(name);
   }

   layout.content_set(groupname+"/progressbar", progslider);

   evas_object_smart_callback_add(emotion._eo_ptr(), "playback_finished", _video_playback_finished_cb, this); //FIXME
   evas_object_smart_callback_add(emotion._eo_ptr(), "frame_decode", _video_frame_decode_cb, this); //FIXME

   progslider.show();
   settings.win.activate();
}

void
videoplayer::opened_done_cb()
{
   player.play();
   evas::object emotion = player.emotion_get();
   evas_object_smart_callback_del(emotion._eo_ptr(), "open_done", _video_open_done_cb); //FIXME
   layout.signal_emit("videoplayer.video.playing", "");
}

void
videoplayer::deactive()
{
   evas::object emotion = player.emotion_get();
   evas_object_smart_callback_del(emotion._eo_ptr(), "playback_finished", _video_playback_finished_cb); //FIXME
   evas_object_smart_callback_del(emotion._eo_ptr(), "frame_decode", _video_frame_decode_cb); //FIXME

   if (player.is_playing_get()) {
     player.stop();
     player.play_position_set(0);
   }

   layout.content_unset(groupname+"/video");
   player.hide();
   progslider.hide();
   basectrl::deactive();
}

void
videoplayer::player_fame_decode_cb()
{
    std::ostringstream label_total, label_pos;
    int h, m, s;

    double pos = player.play_position_get();
    double len = player.play_length_get();

    h = pos / 3600;
    m = pos / 60 - (h * 60);
    s = pos - (h * 3600) - (m * 60);
    label_pos.fill('0');
    if (len / 3600 >= 1)
      label_pos << std::setw(2) << h << ":";
    label_pos << std::setw(2) << m << ":" << std::setw(2) << s;

    h = len / 3600;
    m = len / 60 - (h * 60);
    s = len - (h * 3600) - (m * 60);
    label_total.fill('0');
    if (h) label_total << std::setw(2) << h << ":";
    label_total << std::setw(2) << m << ":" << std::setw(2) << s;

    layout.text_set(groupname+"/curtime", label_pos.str());
    layout.text_set(groupname+"/totaltime", label_total.str());

    progslider.min_max_set(0, len);
    progslider.value_set(pos);
}

} //emc
