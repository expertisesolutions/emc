#ifndef _VIDEOLIST_HH
#define _VIDEOLIST_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"

#include "elm_interface_scrollable.h"
#include "elm_genlist.eo.hh"
#include <elm_slider.eo.hh>
#include <elm_layout.eo.hh>

namespace emc {

class videolist
 : public basectrl
{
   void _on_key_down(std::string key);
   ::elm_genlist genlist;

   //::EModel model;
   public:
     videolist(const ::elm_layout &layout, const std::string &theme);
     ~videolist() {}
     void active();
};

} //emc
#endif
