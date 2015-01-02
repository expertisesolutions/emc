#ifndef _BASECTRL_HH
#define _BASECTRL_HH

#include "settingsmodel.hh"

#include <eo_event.hh>

struct elm_layout;

namespace emc {

class basectrl
{
   std::function<void()> deactive_cb;
   ::efl::eo::signal_connection key_con;

   protected:
     std::string groupname;
     ::elm_layout &layout;
     settingsmodel &settings;

   public:
     basectrl(settingsmodel &settings, const std::string &groupname, const std::function<void()> &deactive_cb);
     ~basectrl() {};
     virtual void active();
     virtual void deactive();
     virtual void on_key_down(std::string key);
     void push(basectrl &ctrl);
};

}

#endif
