#ifndef _EMODEL_HELPER_HH
#define _EMODEL_HELPER_HH

#include <Eina.h>
#include <Eo.h>
#include <Emodel_Common.h>
#include <Emodel.hh>

#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "logger.hh"

namespace emc { namespace emodel_helpers {

template<class T>
inline bool property_get(const ::emodel &model, const std::string &property, T &value)
{
   ::efl::eina::value property_value;
   if (!model.property_get(property, property_value.native_handle()))
     {
        ERR << "Error trying to get " << property << " property";
        return false;
     }

   value = ::efl::eina::get<T>(property_value);
   return true;
}

template<>
inline bool property_get<int64_t>(const ::emodel &model, const std::string &property, int64_t &value)
{
   ::efl::eina::value property_value;
   if (!model.property_get(property, property_value.native_handle()))
     {
        ERR << "Error trying to get " << property << " property";
        return false;
     }

   eina_value_get(property_value.native_handle(), &value);
   return true;
}

template<class T>
inline void property_set(::emodel model, const std::string &property, const T &raw_value)
{
   DBG << "Setting value of " << property << " = " << raw_value;

   ::efl::eina::value value(raw_value);
   model.property_set(property, *value.native_handle());
}

template<class T>
inline std::vector<T> children_get(::emodel model)
{
   std::vector<T> children;

   Eina_Accessor *_ac = nullptr;
   model.children_slice_get(0, 0, &_ac);
   if (nullptr == _ac)
     {
        ERR << "children_slice_get error";
        return children;
     }

   // FIXME: Use EINA-CXX
   Eo *child;
   unsigned int i = 0;
   EINA_ACCESSOR_FOREACH(_ac, i, child)
     {
        T obj(::eo_ref(child));
        children.push_back(obj);
     }
   return children;
}


/**
 * Asynchronously call handler once on load or error
 * @param emodel The emodel
 * @param handler The callback
 */
void async_load(::emodel model, std::function<void(bool)> handler);

/**
 * Asynchronously call handler once on properties load or error
 * @param emodel The emodel
 * @param handler The callback
 */
void async_properties_load(::emodel model, std::function<void(bool)> handler);

/**
 * Register a callback handler to be called just once on properties changed or error
 * @param emodel The emodel
 * @param handler The callback
 */
void callback_properties_changed_once(::emodel model, std::function<void(bool)> handler);

void callback_children_count_changed_add(::emodel model, std::function<bool(bool, unsigned int)> handler);

}}

#endif
