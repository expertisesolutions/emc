#include "logger.hh"

namespace emc {


::efl::eina::log_domain& get_logger()
{
   static ::efl::eina::eina_init init;
   static ::efl::eina::log_domain logger("emc", EINA_COLOR_YELLOW);
   return logger;
}

}
