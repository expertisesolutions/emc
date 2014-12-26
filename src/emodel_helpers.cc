#include "emodel_helpers.hh"

#include <memory>

namespace
{
   bool on_load(void *info, std::function<void()> disconnect, std::function<void(bool)> handler, Emodel_Load_Status status)
   {
       const Emodel_Load &st = *static_cast<Emodel_Load*>(info);
       if (st.status & EMODEL_LOAD_STATUS_ERROR)
         {
            DBG << "Error loading properties";
            disconnect();
            handler(true);
            return false;
         }

       if((st.status & status) != status)
         return true;

       disconnect();

       handler(false);
       return false;
   }

   bool on_properties_changed(void *info, std::function<void()> disconnect, std::function<void(bool)> handler)
   {
      DBG << "Properties changed";
      disconnect();
      handler(false);
      return false;
   }

   bool on_properties_changed_event(void *info, std::function<void()> disconnect, std::function<void(bool, const std::vector<Emodel_Property_Pair*> &)> handler)
   {
      DBG << "Properties changed";
      Emodel_Property_Event &event = *static_cast<Emodel_Property_Event*>(info);
      disconnect();

      std::vector<Emodel_Property_Pair*> properties;

      Eina_List *it = nullptr;
      void *pair = nullptr;
      EINA_LIST_FOREACH(event.changed_properties, it, pair)
        properties.push_back(static_cast<Emodel_Property_Pair*>(pair));

      handler(false, properties);
      return false;
   }

   bool on_properties_changed_error(void *info, std::function<void()> disconnect, std::function<void(bool)> handler)
   {
      const Emodel_Load &st = *static_cast<Emodel_Load*>(info);
      if (!(st.status & EMODEL_LOAD_STATUS_ERROR))
        return true;

      disconnect();
      ERR << "Error";
      handler(true);
      return false;
   }


   bool on_error(void *info, std::function<void()> disconnect, std::function<bool(bool)> handler)
   {
      const Emodel_Load &st = *static_cast<Emodel_Load*>(info);
      if (!(st.status & EMODEL_LOAD_STATUS_ERROR))
        return true;

      ERR << "Error";
      if (!handler(true))
        {
           disconnect();
           return false;
        }
      return true;
   }

   bool on_children_count_changed(void *info, std::function<void()> disconnect, std::function<bool(bool, unsigned int)> handler)
   {
      unsigned int count = *static_cast<unsigned int *>(info);
      if (!handler(false, count))
        {
           disconnect();
           return false;
        }
      return true;
   }
}

namespace emc { namespace emodel_helpers {

void async_load(::emodel model, std::function<void(bool)> handler)
{
   auto status = model.load_status_get();

   if ((status & EMODEL_LOAD_STATUS_LOADED) == EMODEL_LOAD_STATUS_LOADED)
     {
        handler(false);
        return;
     }

   auto connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto disconnect = [connection] { connection->disconnect(); };
   *connection = model.callback_load_status_add(
      std::bind(on_load, std::placeholders::_3, disconnect, handler, EMODEL_LOAD_STATUS_LOADED));

   if ((status & EMODEL_LOAD_STATUS_LOADING) == EMODEL_LOAD_STATUS_LOADING)
     return;

   if (status & EMODEL_LOAD_STATUS_LOADING_PROPERTIES)
     model.children_load();
   else
   if (status & EMODEL_LOAD_STATUS_LOADING_CHILDREN)
     model.properties_load();
   else
     model.load();
}

void async_properties_load(::emodel model, std::function<void(bool)> handler)
{
   if ((model.load_status_get() & EMODEL_LOAD_STATUS_LOADED_PROPERTIES) == EMODEL_LOAD_STATUS_LOADED_PROPERTIES)
     {
        handler(false);
        return;
     }

   auto connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto disconnect = [connection] { connection->disconnect(); };
   *connection = model.callback_load_status_add(
      std::bind(on_load, std::placeholders::_3, disconnect, handler, EMODEL_LOAD_STATUS_LOADED_PROPERTIES));
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

void callback_children_count_changed_add(::emodel model, std::function<bool(bool, unsigned int)> handler)
{
   auto children_count_changed_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto load_status_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto disconnect = [children_count_changed_connection, load_status_connection]
     {
        children_count_changed_connection->disconnect();
        load_status_connection->disconnect();
     };

   *children_count_changed_connection = model.callback_children_count_changed_add(
      std::bind(on_children_count_changed, std::placeholders::_3, disconnect, handler));

   *load_status_connection = model.callback_load_status_add(
      std::bind(on_error, std::placeholders::_3, disconnect,
        [handler](bool error)
        {
           return handler(error, 0u);
        }));
}

template<>
void async_property_set<::efl::eina::value>(::emodel model, const std::string &property, const ::efl::eina::value &value, std::function<void(bool, const std::vector<Emodel_Property_Pair*> &)> handler)
{
   auto properties_changed_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto load_status_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto disconnect = [properties_changed_connection, load_status_connection]
     {
        properties_changed_connection->disconnect();
        load_status_connection->disconnect();
     };

   *properties_changed_connection = model.callback_properties_changed_add(
     std::bind(on_properties_changed_event, std::placeholders::_3, disconnect, handler));

   *load_status_connection = model.callback_load_status_add(
     std::bind(on_properties_changed_error, std::placeholders::_3, disconnect,
       [handler](bool error)
       {
          handler(error, std::vector<Emodel_Property_Pair*>());
       }));

   property_set(model, property, value);
}

}}

