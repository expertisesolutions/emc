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

//Constructor
mainctrl::mainctrl(const ::elm_win &_win, ::elm_layout _layout, const std::string &_theme)
   : basectrl(_win, _layout, _theme, "main")
{
   layout.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(layout);
   win.size_set(WIDTH, HEIGHT);
   win.visibility_set(true);
   win.callback_key_down_add(std::bind([this](void *einfo){key_down_cb(static_cast<Evas_Event_Key_Down *>(einfo)->key);}, std::placeholders::_3));
   std::cout << "Theme file: " << _theme << std::endl;
}

void
mainctrl::key_down_cb(std::string key)
{
   //std::cout << "Main Ctrl Key down "<< key << std::endl;
   if (key == "Right")
     layout.signal_emit("main.show.next", "");
   else if (key == "Left")
     layout.signal_emit("main.show.prev", "");
   else if (key == "Enter")
     layout.signal_emit("main.show.enter", "");
   else if (key == "Esc")
     layout.signal_emit("main.show.exit", "");
}

} //emc
