#ifndef _VIDEOLIST_HH
#define _VIDEOLIST_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"

namespace emc {

class videolist
 : public basectrl
{
   void _on_key_down(std::string key);
   //::EModel model;
   public:
     videolist(const ::elm_layout &layout, const std::string &theme);
     ~videolist() {}
};

} //emc
#endif
