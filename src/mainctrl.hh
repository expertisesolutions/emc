#ifndef _MAINCTRL_HH
#define _MAINCTRL_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"
#include "audiolist.hh"
#include "videolist.hh"
#include "settingsctrl.hh"

namespace emc {

class mainctrl
 : public basectrl
{
   ::elm_win win;
   audiolist audio;
   videolist video;
   settingsctrl settings;

   void _on_key_down(std::string key);

   public:
     mainctrl(const ::elm_win &win, ::elm_layout layout, const std::string &theme);
     ~mainctrl() {}
};

} //emc
#endif
