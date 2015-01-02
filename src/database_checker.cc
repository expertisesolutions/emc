#include "database_checker.hh"

#include "database.hh"
#include "database_schema.hh"
#include "emodel_helpers.hh"
#include "logger.hh"
#include "row_map.hh"

#include <cassert>
#include <ostream>

namespace emc {

database_checker::database_checker(::emc::database &database)
   : database(database)
   , db(database.database_get())
{}

database_checker::~database_checker()
{}

void
database_checker::success()
{
   notify(false);
}

void
database_checker::failure()
{
   notify(true);
}

void
database_checker::notify(bool error)
{
   if (callback)
     callback(error);
   callback = nullptr;
}

void
database_checker::async_check(std::function<void(bool)> handler)
{
   DBG << "Checking tables";

   callback = handler;

   if (database.is_empty())
     {
        migrate(0); // version 0 is an empty database
        return;
     }

   get_version();
}

void
database_checker::migrate(int version)
{
   DBG << "Migrating version from " << version;

   switch(version)
     {
      case 0:
        {
           migrate_from_version_0();
           return;
        }
      case emc::schema::v1::VERSION:
        {
           migrate_from_version_1();
           return;
        }
      case emc::schema::v2::VERSION:
        {
           migrate_from_version_2();
           return;
        }
      case emc::schema::CURRENT_VERSION:
        {
           DBG << "Version " << version << " is the current one. Done!";
           success();
           return;
        }
      default:
        {
           ERR << "Migration procedure not defined";
           failure();
           return;
        }
     }
}

void
database_checker::migrate_from_version_0()
{
   DBG << "Migrating from version 0";

   std::function<void()> migration_done = std::bind(&database_checker::set_version, this, schema::CURRENT_VERSION);

   emc::emodel_helpers::callback_children_count_changed_add(db,
     std::bind(&database_checker::on_table_created, this, std::placeholders::_1, std::placeholders::_2, schema::tables.size(), migration_done));

   for (auto &table : schema::tables)
     create_table(*table);
}

void
database_checker::create_table(const schema::table &table_definition)
{
   DBG << "Creating table: " << table_definition.name;
   efl::eo::base obj = db.child_add();
   esql::model_table table(::eo_ref(obj._eo_ptr()));
   table.name_set(table_definition.name);

   for (auto &field : table_definition.fields)
     create_table_field(table, table_definition, field);
}

void
database_checker::create_table_field(esql::model_table table, const schema::table &table_definition, const schema::field &field_definition)
{
   DBG << "Creating field: " << table_definition.name << "." << field_definition.name;
   ::efl::eina::value field_type(field_definition.type + " " + field_definition.constraint);
   table.property_set(field_definition.name, *field_type.native_handle());
}

bool
database_checker::on_table_created(bool error, unsigned int actual_count, size_t expected_count, std::function<void()> migration_done)
{
   if (error)
     {
        ERR << "Error creating table";
        failure();
        return false;
     }

   if (actual_count != expected_count)
     return true;

   migration_done();
   return false;
}

void
database_checker::set_version(int version)
{
   DBG << "Setting version to " << version;

   auto &version_table = database.version_get();
   emc::emodel_helpers::async_load(version_table, std::bind(&database_checker::on_version_table_loaded, this, std::placeholders::_1, version_table, version));
}

void
database_checker::on_version_table_loaded(bool error, esql::model_table version_table, int version)
{
   if (error)
     {
        ERR << "Error loading version table";
        failure();
        return;
     }

   auto rows = emc::emodel_helpers::children_get<esql::model_row>(version_table);
   assert(rows.size() <= 1);

   if (rows.empty())
     database::async_create_row(version_table, std::bind(&database_checker::on_version_table_row_loaded, this, std::placeholders::_1, std::placeholders::_2, version));
   else
     load_version_row(false, rows.front(), version);
}

void
database_checker::load_version_row(bool error, esql::model_row row, int version)
{
   if (error)
     {
        ERR << "Error creating version table row";
        failure();
        return;
     }

   DBG << "Loading version row";
   emc::emodel_helpers::async_load(row, std::bind(&database_checker::on_version_table_row_loaded, this, std::placeholders::_1, row, version));
}

void
database_checker::on_version_table_row_loaded(bool error, esql::model_row row, int version)
{
   if (error)
     {
        ERR << "Error loading version table row";
        failure();
        return;
     }

   DBG << "Setting version row";
   auto callback = std::bind(&database_checker::on_version_table_row_setted, this, std::placeholders::_1, row, version);
   emc::emodel_helpers::callback_properties_changed_once(row, callback);
   emc::emodel_helpers::property_set(row, "version", version);
}

void
database_checker::on_version_table_row_setted(bool error, esql::model_row row, int version)
{
   if (error)
     {
        ERR << "Error setting version on table row";
        failure();
        return;
     }

   migrate(version);
}

void
database_checker::get_version()
{
   DBG << "Getting database schema version";

   auto version_table = database.table_get(emc::schema::version_table.name);
   if (!version_table)
     {
        migrate(emc::schema::v1::VERSION);
        return;
     }

   auto rows = emc::emodel_helpers::children_get<esql::model_row>(version_table);
   assert(!rows.empty());
   auto row = rows.front();
   emc::emodel_helpers::async_load(row, std::bind(&database_checker::on_version_row_loaded, this, std::placeholders::_1, row));
}

void
database_checker::on_version_row_loaded(bool error, esql::model_row row)
{
   int64_t version;
   if (!emc::emodel_helpers::property_get(row, "version", version))
     {
        ERR << "Error getting database schema version";
        failure();
        return;
     }

   migrate(version);
}

void
database_checker::migrate_from_version_1()
{
   DBG << "Migrating from version 1 to 2";

   for (auto &table : schema::v1::tables)
     assert(database.table_get(table->name));

   auto tracks_table = database.table_get(schema::v2::tracks_table.name);

   auto callback = [this](bool error)
     {
        if (error)
          {
             ERR << "Error creating field";
             failure();
             return;
          }

        create_v2_tables();
     };
   emc::emodel_helpers::callback_properties_changed_once(tracks_table, callback);
   create_table_field(tracks_table, schema::v2::tracks_table, schema::v2::tracks_table.fields[6]);
}

void
database_checker::create_v2_tables()
{
   DBG << "Creating version/settings table";

   emc::emodel_helpers::callback_children_count_changed_add(db,
     std::bind(&database_checker::on_v2_tables_created, this, std::placeholders::_1));
   create_table(schema::v2::version_table);
   create_table(schema::v2::settings_table);
}

bool
database_checker::on_v2_tables_created(bool error)
{
   if (error)
     {
        ERR << "Error creating version table";
        failure();
        return false;
     }

   set_version(schema::v2::VERSION);
   return false;
}

void
database_checker::migrate_from_version_2()
{
   auto tracks_table = database.table_get(schema::v3::tracks_table.name);

   DBG << "Migrating from version 2 to 3";
   auto callback = std::bind(&database_checker::on_migration_completed, this, std::placeholders::_1, schema::v3::VERSION);
   async_create_table_fields(tracks_table, schema::v3::tracks_table, {7, 8, 9, 10}, callback);
}

void
database_checker::on_migration_completed(bool error, int version)
{
   if (error)
     {
        ERR << "Error migrating to version " << version;
        failure();
        return;
     }

   set_version(version);
}

void
database_checker::async_create_table_fields(esql::model_table &table,
                                    const schema::table &table_definition,
                                    const std::vector<int> &field_ids,
                                    std::function<void(bool)> handler)
{
   assert(table);
   assert(pending_properties.empty());
   auto on_property_set = std::bind(&database_checker::on_property_set, this, std::placeholders::_1, std::placeholders::_2, handler);

   for (auto &field_id : field_ids)
     {
        auto field_definition = table_definition.fields[field_id];
        DBG << "Creating field: " << table_definition.name << "." << field_definition.name;
        auto field_type = field_definition.type + " " + field_definition.constraint;
        ::emc::emodel_helpers::async_property_set(table, field_definition.name, field_type, on_property_set);
        pending_properties.insert(field_definition.name);
     }
}

void
database_checker::on_property_set(bool error,
                          const std::vector<Emodel_Property_Pair*> &properties_changed,
                          std::function<void(bool)> handler)
{
   if (error)
     {
        handler(error);
        return;
     }

   assert(!properties_changed.empty());

   for (auto &property_pair : properties_changed)
     {
        assert(property_pair && property_pair->property);
        auto it = pending_properties.find(property_pair->property);
        assert(end(pending_properties) != it);
        pending_properties.erase(it);
        DBG << "Field " << property_pair->property << " created";
     }

   if (pending_properties.empty())
     handler(error);
}

}
