/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

//#include "database_schema.hh"
//#include "emodel_helpers.hh"
//#include "logger.hh"

//#include <functional>
//#include <utility>
#include <Evas.h>
#include <Elementary.h>
#include <elm_widget.h>
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"

#include "playlistmodel.hh"
#include "emodel_helpers.hh"
#include <eina_accessor.hh>
#include <Ecore.hh>
#include <thread>

namespace emc {

//FIXME
static void
playback_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
   playlistmodel *t = static_cast<playlistmodel*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }
   efl::ecore::main_loop_thread_safe_call_async(std::bind(&playlistmodel::play_next, t));
}

//FIXME
static void
playlist_open_done_cb(void *data, Evas_Object *obj, void *event_info)
{
   playlistmodel *t = static_cast<playlistmodel*>(data);
   if(!t)
     {
        std::string errmsg("Invalid Data\n");
        std::cerr << "Error: " << errmsg;
        return;
     }

   efl::ecore::main_loop_thread_safe_call_async(std::bind(&playlistmodel::opened_done_cb, t));
}

playlistmodel::playlistmodel(settingsmodel &_settings)
        : inherit_base(efl::eo::parent = nullptr),
        settings (_settings),
        player(settings.player)
{
    evas::object emotion = player.emotion_get();
    evas_object_smart_callback_add(emotion._eo_ptr(), "playback_finished", playback_finished_cb, this); //FIXME
}

void
playlistmodel::list_set(std::vector<esql::model_row> tracks_)
{
   tracks.clear();
   for (auto &track : tracks_)
     {
        tracks.push_back(track);
     }
}

void
playlistmodel::track_add(esql::model_row track)
{
   tracks.push_back(track);
}

Emodel_Load_Status
playlistmodel::children_slice_get(unsigned start, unsigned count, Eina_Accessor ** children_accessor)
{
   *children_accessor = eina_list_accessor_new(tracks.native_handle());
   return EMODEL_LOAD_STATUS_LOADED;
}

Emodel_Load_Status
playlistmodel::children_count_get(unsigned* children_count)
{
   *children_count = tracks.size();
   return EMODEL_LOAD_STATUS_LOADED;
}

void
playlistmodel::play(esql::model_row track)
{
   play(std::find(tracks.begin(), tracks.end(), track));
}

esql::model_row
playlistmodel::play(efl::eina::ptr_list<esql::model_row>::iterator track)
{
   std::string path;
   if (emc::emodel_helpers::property_get(*track, "file", path)) {
     evas::object emotion = player.emotion_get();
     evas_object_smart_callback_add(emotion._eo_ptr(), "open_done", playlist_open_done_cb, this); //FIXME
     player.file_set(path, "");
     current_track = track;
   }
   return *current_track;
}

esql::model_row
playlistmodel::play()
{
   if (player.play_position_get() == 0)
      return play(current_track);

   player.play();
   return *current_track;
}

void
playlistmodel::pause()
{
   player.pause();
}

esql::model_row
playlistmodel::play_next()
{
   auto next = std::next(current_track);
   if (next != tracks.end())
     return play(next);

   return *current_track;
}

esql::model_row
playlistmodel::play_prev()
{
   if (current_track != tracks.begin())
     return play(std::prev(current_track));

   return *current_track;
}

esql::model_row
playlistmodel::curr()
{
   return *current_track;
}

void
playlistmodel::opened_done_cb()
{
   player.play();
   evas::object emotion = player.emotion_get();
   evas_object_smart_callback_del(emotion._eo_ptr(), "open_done", playlist_open_done_cb); //FIXME
}

} //emc
