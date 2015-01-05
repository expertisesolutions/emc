#ifndef _MAINCTRL_HH
#define _MAINCTRL_HH

#include "audiolist.hh"
#include "basectrl.hh"
#include "settingsctrl.hh"
#include "videolist.hh"

#include <functional>

namespace emc {

class database;
class settingsmodel;

class mainctrl : public basectrl
{
   audiolist audio;
   videolist video;
   settingsctrl settctrl;

   public:
     mainctrl(::emc::database &database,
              settingsmodel &settings,
              std::function<void()> start_tagging);
     ~mainctrl() {}
     void on_key_down(std::string key);
};

}

#endif
