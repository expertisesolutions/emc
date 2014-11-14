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

#include <Ecore.hh>
#include "videoplayer.hh"

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

videoplayer::videoplayer(const settingsmodel &settings, const std::function<void()> &deactive_cb)
   : basectrl(settings, "videoplayer", deactive_cb),
        player(settings.player),
        progslider(efl::eo::parent = layout)
{
}

void
videoplayer::play(emodel model)
{
   Eina_Value v;

   if (player.is_playing_get())
     player.stop();

   layout.content_set(groupname+"/video", player);
   player.show();

   model.property_get("path", &v);
   char *path = eina_value_to_string(&v);
   if (path) {
     player.file_set(path, "");
     player.play();
     layout.signal_emit("videoplayer.video.playing", "");
     free(path);
   }

   layout.content_set(groupname+"/progressbar", progslider);

   evas::object emotion = player.emotion_get();
   evas_object_smart_callback_add(emotion._eo_ptr(), "playback_finished", _video_playback_finished_cb, this); //FIXME
   evas_object_smart_callback_add(emotion._eo_ptr(), "frame_decode", _video_frame_decode_cb, this); //FIXME

   progslider.show();
   eo_unref(progslider._eo_ptr()); //XXX
   settings.win.activate();
}

void
videoplayer::deactive()
{
   evas::object emotion = player.emotion_get();
   evas_object_smart_callback_del(emotion._eo_ptr(), "playback_finished", _video_playback_finished_cb); //FIXME
   evas_object_smart_callback_del(emotion._eo_ptr(), "frame_decode", _video_frame_decode_cb); //FIXME

   if (player.is_playing_get())
     player.stop();

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

    label_pos << m << ":" << s;

    h = len / 3600;
    m = len / 60 - (h * 60);
    s = len - (h * 3600) - (m * 60);

    label_total << m << ":" << s;

    layout.text_set(groupname+"/curtime", label_pos.str());
    layout.text_set(groupname+"/totaltime", label_total.str());

    progslider.min_max_set(0, len);
    progslider.value_set(pos);
}


} //emc
