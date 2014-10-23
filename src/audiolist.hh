#ifndef _AUDIOLIST_HH
#define _AUDIOLIST_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"
#include "settingsmodel.hh"
#include "audiolistmodel.hh"

#include "elm_interface_scrollable.h"
#include "elm_genlist.eo.hh"
#include <elm_slider.eo.hh>
#include <elm_layout.eo.hh>
#include "elm_view_list.eo.hh"

namespace emc {

class audiolist
 : public basectrl
{
   void _on_key_down(std::string key);
   ::elm_genlist list;
   ::elm_view_list view;
   audiolistmodel model;
   settingsmodel settings;

   public:
     audiolist(const ::elm_layout &layout, const std::string &theme, settingsmodel &settings);
     ~audiolist() {}
     void active();
     void deactive();
};

} //emc
#endif
