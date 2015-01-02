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

   // initialize services
   ::emc::database database;
   ::emc::database_map database_map(database);
   ::emc::tagging_service tagging_service(database, database_map);

   // start services and connect event handlers
   auto on_database_mapped =
     [&tagging_service]()
     {
        tagging_service.start();
     };

   DBG << "Loading database...";
   database.async_load(
     [&database_map, on_database_mapped](bool error)
     {
        if (error)
          {
             ERR << "Error loading database";
             elm_exit();
             return;
          }

        DBG << "Mapping database rows";
        database_map.async_map(on_database_mapped);
     });

   // initialize gui
   ::elm_win win(elm_win_util_standard_add("emc-window","Enlightenment Media Center - EMC"));
   ::elm_layout layout(efl::eo::parent = win);
   win.callback_delete_request_add(std::bind([]{elm_exit();}));

   emc::settingsmodel settings(database, database_map, win, layout);
   emc::mainctrl mctrl(database, settings);
   mctrl.active();

   elm_run();
   win._release();

   return EXIT_SUCCESS;
}

}
