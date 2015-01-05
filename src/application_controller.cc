#include "application_controller.hh"

#include "database.hh"
#include "database_map.hh"
#include "logger.hh"
#include "mainctrl.hh"
#include "settingsmodel.hh"
#include "tagging_service.hh"

#include <Elementary.h>
#include <elm_widget.h>
#include <elm_layout.eo.hh>
#include <elm_win.eo.hh>

#include <cstdlib>

namespace emc {

application_controller::application_controller()
{
}

application_controller::~application_controller()
{
}

int
application_controller::run()
{
   // basic initializations
   if (!esql_init())
     {
        ERR << "Could not initialize Esql" << std::endl;
        return EXIT_FAILURE;
     }

   // initialize gui
   ::elm_win win(elm_win_util_standard_add("emc-window","Enlightenment Media Center - EMC"));
   ::elm_layout layout(efl::eo::parent = win);
   win.callback_delete_request_add(std::bind([]{elm_exit();}));

   // initialize services
   ::emc::database database;
   ::emc::database_map database_map(database);
   ::emc::tagging_service tagging_service(database, database_map);

   emc::settingsmodel settings(database, database_map, win, layout);

   // start services and connect event handlers
   auto start_tagging =
     [&tagging_service, &settings]()
     {
        auto audio_path = settings.audio_rootpath_get();
        tagging_service.scan(audio_path);
     };

   DBG << "Loading database...";
   database.async_load(
     [&database_map, start_tagging](bool error)
     {
        if (error)
          {
             ERR << "Error loading database";
             elm_exit();
             return;
          }

        DBG << "Mapping database rows";
        database_map.async_map(start_tagging);
     });

   emc::mainctrl mctrl(database, settings, start_tagging);
   mctrl.active();

   elm_run();
   win._release();

   return EXIT_SUCCESS;
}

}
