#include "tag_processor.hh"

#include "emodel_helpers.hh"

namespace {
   bool
   exists(const std::unordered_map<std::string, esql::model_row> &map,
          const std::string &key)
   {
      auto it = map.find(key);
      return end(map) != it;
   }

   void async_create_row(esql::model_table &table, std::function<void(bool, esql::model_row)> callback)
   {
      auto obj = table.child_add();
      std::cout << "Row created: " << obj._eo_ptr() << std::endl;
      esql::model_row row(::eo_ref(obj._eo_ptr()));
      emc::emodel_helpers::async_properties_load(row, std::bind(callback, std::placeholders::_1, row));
   }
}

namespace emc {

tag_processor::tag_processor(std::unordered_map<std::string, esql::model_row> &map,
                             esql::model_table &table,
                             const std::string &key_value,
                             std::function<void()> process_next,
                             std::function<void(esql::model_row)> property_set)
   : map(map)
   , table(table)
   , key_value(key_value)
   , process_next(process_next)
   , property_set(property_set)
{
    std::cout << "Tag processor ctor..." << std::endl;
}

tag_processor::~tag_processor()
{
   std::cout << "Tag processor dtor..." << std::endl;
}

void
tag_processor::process()
{
   std::cout << "Tag processor: " << key_value << std::endl;
   if (exists(map, key_value))
     {
        process_next();
        return;
     }

   std::cout << "Inserting: " << key_value << std::endl;
   async_create_row(table, std::bind(&tag_processor::row_create_handler, this, std::placeholders::_1, std::placeholders::_2));
   return;
}

void
tag_processor::row_create_handler(bool error, esql::model_row row)
{
   if (error)
     {
        std::cout << "Error creating row, continuing..." << std::endl;
        process_next();
        return;
     }

   std::cout << "Row created..." << std::endl;
   auto callback = std::bind(&tag_processor::property_set_handler, this, std::placeholders::_1, row);
   emc::emodel_helpers::callback_properties_changed_once(row, callback);
   property_set(row);
}

void
tag_processor::property_set_handler(bool error, esql::model_row row)
{
   if (error)
     std::cout << "Error setting properties, continuing..." << std::endl;
   else
     {
        std::cout << "Properties have been set for '" << key_value << "', continuing..." << std::endl;
        map.insert(std::make_pair(key_value, row));
     }

   process_next();
}

}

