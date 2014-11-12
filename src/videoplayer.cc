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

#include "videoplayer.hh"

namespace emc {

videoplayer::videoplayer(const settingsmodel &settings, const std::function<void()> &deactive_cb)
   : basectrl(settings, "videoplayer", deactive_cb),
        player(efl::eo::parent = layout),
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
}

void
videoplayer::deactive()
{
   if (player.is_playing_get())
     player.stop();

   layout.content_unset(groupname+"/video");
   player.hide();
   basectrl::deactive();
}

} //emc
