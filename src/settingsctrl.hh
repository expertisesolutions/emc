#ifndef _SETTINGS_CTRL_HH
#define _SETTINGS_CTRL_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"
#include "settingsmodel.hh"

namespace emc {

class settingsctrl
 : public basectrl
{
   void _on_key_down(std::string key);
   settingsmodel settings;

   public:
     settingsctrl(const ::elm_layout &layout, const std::string &theme, settingsmodel &settings);
     ~settingsctrl() {}
};

} //emc
#endif
