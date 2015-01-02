#include "application_controller.hh"

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   ::emc::application_controller application;
   return application.run();
}
ELM_MAIN()
