#ifndef _SETTINGS_CTRL_HH
#define _SETTINGS_CTRL_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"
#include "settingsmodel.hh"
#include <elm_fileselector_entry.eo.hh>

namespace emc {

class settingsctrl
 : public basectrl
{
   ::elm_fileselector_entry m_fentry;
   ::elm_fileselector_entry v_fentry;

   public:
     settingsctrl(const settingsmodel &settings, const std::function<void()> &_cb);
     ~settingsctrl() {}

     void active();
     void deactive();
};

} //emc
#endif
