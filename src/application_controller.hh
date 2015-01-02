#ifndef _APPLICATION_CONTROLLER_HH
#define _APPLICATION_CONTROLLER_HH

namespace emc {

class database_map;

/**
 * Wires all dependencies to run the application.
 */
class application_controller
{
public:
   application_controller();
   ~application_controller();

   /**
    * Runs the application. Blocks until finished.
    * @return Returns EXIT_FAILURE on failure or EXIT_SUCCESS on success
    */
   int run();

};

}

#endif
