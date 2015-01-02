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

   bool on_load_status_error(void *info, std::function<void()> disconnect, std::function<void(bool)> handler)
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

   void
   on_child_loaded(bool error, std::shared_ptr<int> count, std::function<void(bool)> handler)
   {
      --(*count);

      if (error)
        {
           ERR << "Error loading child";
           handler(true);
           return;
        }

      if (*count) return;

      handler(false);
   }

   void
   on_child_added(void *info, std::function<void(const Emodel_Children_Event&)> handler)
   {
      const Emodel_Children_Event &event = *static_cast<Emodel_Children_Event*>(info);
      handler(event);
   }

   void
   on_child_removed(void *info, std::function<void()> disconnect, std::shared_ptr<int> count, std::function<void(bool)> handler)
   {
      DBG << "Child removed: " << *count;
      --(*count);

      if (!(*count))
        {
           disconnect();
           handler(false);
        }
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

void async_children_load(::emodel model, std::function<void(bool)> handler)
{
   auto children = emc::emodel_helpers::children_get<::emodel>(model);
   if (children.empty())
     {
        handler(false);
        return;
     }

   auto count = std::make_shared<int>(children.size());
   for (auto &child : children)
     emc::emodel_helpers::async_load(child, std::bind(&on_child_loaded, std::placeholders::_1, count, handler));
}

void async_child_del(::emodel model, const std::vector<::emodel> &children, std::function<void(bool)> handler)
{
   assert(!children.empty());

   auto count = std::make_shared<int>(children.size());

   auto child_removed_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto load_status_connection = std::make_shared<::efl::eo::signal_connection>(nullptr);
   auto disconnect = [child_removed_connection, load_status_connection]
     {
        child_removed_connection->disconnect();
        load_status_connection->disconnect();
     };

   *child_removed_connection = model.callback_child_removed_add(
      std::bind(on_child_removed, std::placeholders::_3, disconnect, count, handler));

   *load_status_connection = model.callback_load_status_add(
      std::bind(on_load_status_error, std::placeholders::_3, disconnect, handler));

   for (auto &child : children)
     model.child_del(child);
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
      std::bind(on_load_status_error, std::placeholders::_3, disconnect, handler));
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
     std::bind(on_load_status_error, std::placeholders::_3, disconnect,
       [handler](bool error)
       {
          handler(error, std::vector<Emodel_Property_Pair*>());
       }));

   property_set(model, property, value);
}


::efl::eo::signal_connection
child_added_event_connect(::emodel model, std::function<void(const Emodel_Children_Event&)> handler)
{
   return model.callback_child_added_add(
     std::bind(on_child_added, std::placeholders::_3, handler));
}

}}

