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
   elm_object_focus_set(win._eo_ptr(), EINA_FALSE);
   win.resize_object_add(layout);
   win.size_set(WIDTH, HEIGHT);
   win.visibility_set(true);
   std::cout << "Theme file: " << _theme << std::endl;
}

} //emc
