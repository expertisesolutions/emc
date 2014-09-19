#ifndef _SETTINGS_CTRL_HH
#define _SETTINGS_CTRL_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"

namespace emc {

class settingsctrl
 : public basectrl
{
   void _on_key_down(std::string key);
   //::EModel model;
   public:
     settingsctrl(const ::elm_layout &layout, const std::string &theme);
     ~settingsctrl() {}
};

} //emc
#endif
