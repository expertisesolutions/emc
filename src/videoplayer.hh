#ifndef _EMC_HH
#define _EMC_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"

#include <elm_slider.eo.hh>
#include <elm_layout.eo.hh>
#include <elm_video.eo.hh>
#include "Emodel.hh"

namespace emc {

class videoplayer
 : public basectrl
{
   ::elm_video player;
   ::elm_slider progslider;

   public:
     videoplayer(const settingsmodel &settings, const std::function<void()> &deactive_cb);
     ~videoplayer() {}

     void play(emodel model);
     void deactive();
     void opened_done_cb();
     void player_fame_decode_cb();
};

} //emc
#endif
