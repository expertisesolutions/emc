#ifndef _VIDEOLIST_HH
#define _VIDEOLIST_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"
#include "settingsmodel.hh"
#include "videoplayer.hh"

#include "elm_interface_scrollable.h"
#include "elm_genlist.eo.hh"
#include <elm_slider.eo.hh>
#include <elm_layout.eo.hh>
#include "Eio.h"
#include "Emodel.h"
#include "eio_model.eo.hh"
#include "elm_view_list.eo.hh"

namespace emc {

class videolist
 : public basectrl
{
   void _on_key_down(std::string key);
   ::elm_view_list view;
   ::elm_genlist list;
   eio::model selected;
   videoplayer vp;

   //::EModel model;
   public:
     videolist(settingsmodel &settings, const std::function<void()> &deactive_cb);
     ~videolist() {}
     void active();
     void deactive();
     void push_back();
     void list_activated_cb();
};

} //emc
#endif
