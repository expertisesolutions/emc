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
   public:
     settingsctrl(const settingsmodel &settings, const std::function<void()> &_cb);
     ~settingsctrl() {}
};

} //emc
#endif
