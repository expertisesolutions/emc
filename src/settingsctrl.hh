#ifndef _SETTINGS_CTRL_HH
#define _SETTINGS_CTRL_HH

#include <iostream>

extern "C"
{
#include "elm_interface_atspi_text.h"
}
#include <Elementary.h>

#include "basectrl.hh"
#include "settingsmodel.hh"

#include <elm_fileselector_entry.eo.hh>
#include <elm_check.eo.hh>
#include <elm_button.eo.hh>

namespace emc {

class settingsctrl
 : public basectrl
{
   ::elm_fileselector_entry m_fentry;
   ::elm_fileselector_entry v_fentry;
   ::elm_check fullscreen_check;
   ::elm_button mupdate_bt;

   public:
     settingsctrl(settingsmodel &settings, const std::function<void()> &_cb);
     ~settingsctrl() {}

     void active();
     void deactive();
};

} //emc
#endif
