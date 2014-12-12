#ifndef _LOGGER_HH
#define _LOGGER_HH

#include <Eina.hh>

namespace emc {

::efl::eina::log_domain& get_logger();

}

#define DBG EINA_CXX_DOM_LOG_DBG(::emc::get_logger())
#define INF EINA_CXX_DOM_LOG_INFO(::emc::get_logger())
#define WRN EINA_CXX_DOM_LOG_WARN(::emc::get_logger())
#define ERR EINA_CXX_DOM_LOG_ERR(::emc::get_logger())
#define CRI EINA_CXX_DOM_LOG_CRIT(::emc::get_logger())

#endif
