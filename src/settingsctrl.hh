#ifndef _SETTINGS_CTRL_HH
#define _SETTINGS_CTRL_HH

#include "basectrl.hh"

#include <Elementary.h>
#include <elm_fileselector_entry.eo.hh>
#include <elm_check.eo.hh>
#include <elm_button.eo.hh>

namespace emc {

class database;
class settingsmodel;

class settingsctrl : public basectrl
{
public:
   settingsctrl(::emc::database &database,
                settingsmodel &settings,
                std::function<void()> start_tagging,
                const std::function<void()> &_cb);
   ~settingsctrl() {}

   void active();
   void deactive();

private:
   void update_media();

private:
   ::emc::database &database;
   std::function<void()> start_tagging;
   ::elm_fileselector_entry m_fentry;
   ::elm_fileselector_entry v_fentry;
   ::elm_check fullscreen_check;
   ::elm_button mupdate_bt;
};

}

#endif
