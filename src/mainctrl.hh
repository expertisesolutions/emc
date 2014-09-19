#ifndef _MAINCTRL_HH
#define _MAINCTRL_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"

namespace emc {

class mainctrl
 : public basectrl
{
   void _on_key_down(std::string key);
   //::EModel model;
   public:
     mainctrl(const ::elm_win &win, ::elm_layout layout, const std::string &theme);
     ~mainctrl() {}
};

} //emc
#endif
