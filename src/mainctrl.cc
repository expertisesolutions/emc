/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include "mainctrl.hh"

#include "settingsmodel.hh"

#include <Evas.h>
#include <Elementary.h>
#include <elm_widget.h>
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"
#include <elm_layout.eo.hh>

#include <iostream>

namespace emc {

void
mainctrl::on_key_down(std::string key)
{
   std::cout << "Main Ctrl Key down "<< key << std::endl;
   if (key == "Return")
     layout.signal_emit("main.select.current", "");
   else if (key == "Right")
     layout.signal_emit("main.show.next", "");
   else if (key == "Left")
     layout.signal_emit("main.show.prev", "");
   else
     basectrl::on_key_down(key);
}

mainctrl::mainctrl(::emc::database &database,
                   settingsmodel &_settings,
                   std::function<void()> start_tagging)
   : basectrl(_settings, "main", []{elm_exit();})
   , settctrl(database, settings, start_tagging, [this]{active();})
   , audio(database, settings, [this]{active();})
   , video(settings, [this]{active();})
{
   //add signal callbacks
   layout.signal_callback_add("main.selected.audio", "*",
      std::bind([this]
        {
          push(audio);
          std::cout << "audio selected cb" << std::endl;
        }
      ));

   layout.signal_callback_add("main.selected.video", "*",
      std::bind([this]
        {
          push(video);
          std::cout << "video selected cb" << std::endl;
        }
      ));

   layout.signal_callback_add("main.selected.settings", "*",
      std::bind([this]
        {
          push(settctrl);
          std::cout << "settings selected cb" << std::endl;
        }
      ));
}

} //emc
