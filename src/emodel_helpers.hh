#ifndef _EMODEL_HELPER_HH
#define _EMODEL_HELPER_HH

#include <Eina.h>
#include <Eo.h>
#include <Emodel_Common.h>
#include <Emodel.hh>

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <functional>

namespace emc { namespace emodel_helpers {

template<class T>
inline bool property_get(const ::emodel &model, const std::string &property, T &value)
{
   ::efl::eina::value property_value;
   if (!model.property_get(property, property_value.native_handle()))
     {
        std::cout << "Error trying to get " << property << " property" << std::endl;
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
        std::cout << "Error trying to get " << property << " property" << std::endl;
        return false;
     }

   eina_value_get(property_value.native_handle(), &value);
   return true;
}

template<class T>
inline void property_set(::emodel model, const std::string &property, const T &raw_value)
{
   std::cout << "Setting value of " << property << " = " << raw_value << std::endl;

   ::efl::eina::value value(raw_value);
   model.property_set(property, *value.native_handle());
}

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

}}

#endif
