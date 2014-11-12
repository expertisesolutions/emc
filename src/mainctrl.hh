#ifndef _MAINCTRL_HH
#define _MAINCTRL_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"
#include "audiolist.hh"
#include "videolist.hh"
#include "settingsctrl.hh"
#include "settingsmodel.hh"

namespace emc {

class mainctrl
 : public basectrl
{
   audiolist audio;
   videolist video;
   settingsctrl settctrl;

   public:
     mainctrl(settingsmodel &settings);
     ~mainctrl() {}
     void on_key_down(std::string key);
};

} //emc
#endif
