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
#include <elm_video.eo.hh>
#include "elm_view_list.eo.hh"

namespace emc {

class audiolist
 : public basectrl
{
   void _on_key_down(std::string key);
   std::function<void(esql::model_table&)> _model_set;
   ::elm_genlist list;
   ::elm_view_list view;
   ::elm_video player;
   esql::model_row row_selected;
   audiolistmodel model;
   settingsmodel settings;
   void artists_show(esql::model_table& model);
   void albums_show(esql::model_table& model);
   void playlists_show(esql::model_table& model);

   public:
     audiolist(const ::elm_layout &layout, const std::string &theme, settingsmodel &settings);
     ~audiolist() {}
     void active();
     void deactive();
     void player_playback_finished_cb();
     void player_fame_decode_cb();
     void list_activated_cb();
};

} //emc
#endif
