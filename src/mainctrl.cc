/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include <iostream>

#include <Evas.h>
#include <Elementary.h>
#include <elm_widget.h>
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"
#include <elm_layout.eo.hh>

#include "mainctrl.hh"

#define WIDTH 800
#define HEIGHT 600

namespace emc {

void
mainctrl::_on_key_down(std::string key)
{
   std::cout << "Main Ctrl Key down "<< key << std::endl;
   if (key == "Return")
     layout.signal_emit("main.select.current", "");
   else if (key == "Right")
     layout.signal_emit("main.show.next", "");
   else if (key == "Left")
     layout.signal_emit("main.show.prev", "");
   else if (key == "Escape")
     layout.signal_emit("main.show.exit", "");
}

//Constructor
mainctrl::mainctrl(const ::elm_win &_win, ::elm_layout &_layout, const std::string &_theme)
   : basectrl(_layout, _theme, "main"),
     set_model(),
     settings(_layout, _theme, set_model),
     audio(_layout, _theme, set_model),
     video(_layout, _theme, set_model),
     win(_win)
{
   layout.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(layout);
   win.size_set(WIDTH, HEIGHT);
   win.visibility_set(true);
   layout.visibility_set(true);
   win.callback_key_down_add(std::bind([this](void *einfo){_on_key_down(static_cast<Evas_Event_Key_Down *>(einfo)->key);}, std::placeholders::_3));
   audio.deactive_cb_set([this]{active();});
   video.deactive_cb_set([this]{active();});
   settings.deactive_cb_set([this]{active();});

   deactive_cb_set([]{elm_exit();});

   //add signal callbacks
   layout.signal_callback_add("main.selected.audio", "*",
      std::bind([this]
        {
          audio.active();
          std::cout << "audio selected cb" << std::endl;
        }
      ));

   layout.signal_callback_add("main.selected.video", "*",
      std::bind([this]
        {
          video.active();
          std::cout << "video selected cb" << std::endl;
        }
      ));

   layout.signal_callback_add("main.selected.settings", "*",
      std::bind([this]
        {
          settings.active();
          std::cout << "settings selected cb" << std::endl;
        }
      ));

   layout.signal_callback_add("main.selected.exit", "*",
      std::bind([this]
        {
          deactive();
        }
      ));

   std::cout << "Theme file: " << _theme << std::endl;
}

} //emc
