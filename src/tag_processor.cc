#include "tag_processor.hh"

#include "database.hh"
#include "emodel_helpers.hh"
#include "logger.hh"

namespace emc {

tag_processor::tag_processor(row_map &map,
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
    DBG << "Tag processor ctor...";
}

tag_processor::~tag_processor()
{
   DBG << "Tag processor dtor...";
}

bool
tag_processor::process()
{
   DBG << "Tag processor: " << key_value;
   if (map.exists(key_value))
     return false;

   DBG << "Inserting: " << key_value;
   database::async_create_row(table, std::bind(&tag_processor::row_create_handler, this, std::placeholders::_1, std::placeholders::_2));
   return true;
}

void
tag_processor::row_create_handler(bool error, esql::model_row row)
{
   if (error)
     {
        ERR << "Error creating row, continuing...";
        process_next();
        return;
     }

   DBG << "Row created...";
   auto callback = std::bind(&tag_processor::property_set_handler, this, std::placeholders::_1, row);
   emc::emodel_helpers::callback_properties_changed_once(row, callback);
   property_set(row);
}

void
tag_processor::property_set_handler(bool error, esql::model_row row)
{
   if (error)
     ERR << "Error setting properties, continuing...";
   else
     {
        DBG << "Properties have been set for '" << key_value << "', continuing...";
        map.add(key_value, row);
     }

   process_next();
}

}

