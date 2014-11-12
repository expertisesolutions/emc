#ifndef _BASECTRL_HH
#define _BASECTRL_HH

#include <iostream>

#include <elm_layout.eo.hh>
#include "settingsmodel.hh"

namespace emc {

class basectrl
{
   std::function<void()> deactive_cb;
   ::efl::eo::signal_connection key_con;

   protected:
     std::string groupname;
     settingsmodel settings;
     ::elm_layout layout;

   public:
     basectrl(const settingsmodel &settings, const std::string &groupname, const std::function<void()> &deactive_cb);
     ~basectrl() {};
     virtual void active();
     virtual void deactive();
     virtual void on_key_down(std::string key);
     void push(basectrl &ctrl);
};

} //emc
#endif
