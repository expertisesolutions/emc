#include "emodel_helpers.hh"

#include <memory>

namespace
{
   bool on_properties_load(void *info, std::function<void()> disconnect, std::function<void(bool)> handler)
   {
       const Emodel_Load &st = *static_cast<Emodel_Load*>(info);
       std::cout << __LINE__ << std::endl;
       if (st.status & EMODEL_LOAD_STATUS_ERROR)
         {
            std::cout << "Error loading properties" << std::endl;
            disconnect();
            handler(true);
            return false;
         }

       if(!(st.status & EMODEL_LOAD_STATUS_LOADED_PROPERTIES))
         return true;

       disconnect();

       handler(false);
       return false;
   }

   bool on_properties_changed(void *info, std::function<void()> disconnect, std::function<void(bool)> handler)
   {
      disconnect();
      handler(false);
      return false;
   }

   bool on_properties_changed_error(void *info, std::function<void()> disconnect, std::function<void(bool)> handler)
   {
      const Emodel_Load &st = *static_cast<Emodel_Load*>(info);
      if (!(st.status & EMODEL_LOAD_STATUS_ERROR))
        return true;

      disconnect();
      std::cout << "Error setting property" << std::endl;
      handler(true);
      return false;
   }
}

namespace emc { namespace emodel_helpers {

void async_properties_load(::emodel model, std::function<void(bool)> handler)
{
   auto connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto disconnect = [connection] { connection->disconnect(); };
   *connection = model.callback_load_status_add(
      std::bind(on_properties_load, std::placeholders::_3, disconnect, handler));
   model.properties_load();
}

void callback_properties_changed_once(::emodel model, std::function<void(bool)> handler)
{
   auto properties_changed_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto load_status_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto disconnect = [properties_changed_connection, load_status_connection]
     {
        properties_changed_connection->disconnect();
        load_status_connection->disconnect();
     };

   *properties_changed_connection = model.callback_properties_changed_add(
      std::bind(on_properties_changed, std::placeholders::_3, disconnect, handler));

   *load_status_connection = model.callback_load_status_add(
      std::bind(on_properties_changed_error, std::placeholders::_3, disconnect, handler));
}

}}

