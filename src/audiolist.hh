#ifndef _AUDIOLIST_HH
#define _AUDIOLIST_HH

#include <iostream>
#include <Elementary.h>

#include "basectrl.hh"
#include "audiolistmodel.hh"

#include "elm_interface_scrollable.h"
#include "elm_genlist.eo.hh"
#include <elm_slider.eo.hh>
#include <elm_layout.eo.hh>
#include <elm_video.eo.hh>
#include "elm_view_list.eo.hh"


#include "playlistmodel.hh"

namespace emc {

class audiolist
 : public basectrl
{
   void _on_key_down(std::string key);
   std::function<void(esql::model_table&)> _model_set;
   ::elm_genlist list;
   ::elm_view_list view;
   ::elm_slider progslider;
   esql::model_row row_selected;
   audiolistmodel model;
   playlistmodel playlist;
   void artists_show(esql::model_table& model);
   void albums_show(esql::model_table& model);
   void playlists_show(esql::model_table& model);

   public:
     audiolist(::emc::database &database, settingsmodel &settings, const std::function<void()> &deactive_cb);
     ~audiolist() {}
     void active();
     void deactive();
     void player_fame_decode_cb();
     void list_activated_cb();
     void playback_update();
};

} //emc
#endif
