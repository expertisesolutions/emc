#include "database_schema.hh"

#include <ostream>

extern "C"
{
#include <Esskyuehl.h>
#include <Esql_Model.h>
}
#include <Esql_Model.hh>

namespace emc { namespace schema {

void create_table(const schema::table &table_definition, esql::model &database)
{
   std::cout << "Creating table: " << table_definition.name << std::endl;
   efl::eo::base obj = database.child_add();
   esql::model_table table(::eo_ref(obj._eo_ptr()));
   table.name_set(table_definition.name);

   for (auto &field : table_definition.fields)
     {
        std::cout << "Creating field: " << table_definition.name << "." << field.name << std::endl;
        ::efl::eina::value field_type(field.type + " " + field.constraint);
        table.property_set(field.name, *field_type.native_handle());
     }
}

void create_database(esql::model &database)
{
   for (auto &table : schema::tables)
     create_table(*table, database);
}

}}
