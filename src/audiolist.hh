#ifndef _AUDIOLIST_HH
#define _AUDIOLIST_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"

namespace emc {

class audiolist
 : public basectrl
{
   void _on_key_down(std::string key);
   //::EModel model;
   public:
     audiolist(const ::elm_layout &layout, const std::string &theme);
     ~audiolist() {}
};

} //emc
#endif
