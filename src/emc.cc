#include <stdio.h>
#include <iostream>

#ifdef HAVE_CONFIG_H
# include <elementary_config.h>
#endif

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>
#include <Eina.hh>

#include <elm_widget.h>
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"

#include <elm_win.eo.hh>
#include <elm_layout.eo.hh>

#include "basectrl.hh"
#include "mainctrl.hh"
#include "settingsmodel.hh"

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   //elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   ::elm_win win(elm_win_util_standard_add("emc-window","Enlightenment Media Center - EMC"));
   //win.autodel_set(true);
   ::elm_layout layout(efl::eo::parent = win);
   win.callback_delete_request_add(std::bind([]{elm_exit();}));

   emc::settingsmodel settings(win, layout);
   emc::mainctrl mctrl(settings);
   mctrl.active();

   elm_run();

   return 0;
}
ELM_MAIN()
