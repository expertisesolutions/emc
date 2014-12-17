#ifndef _DATABASE_SCHEMA_HH
#define _DATABASE_SCHEMA_HH

#include <functional>
#include <string>
#include <vector>

namespace esql { struct model; }

namespace emc { namespace schema {

struct field
{
   std::string name;
   std::string type;
   std::string constraint;
};

struct table
{
   std::string name;
   std::vector<field> fields;
};

// SQLite types
const std::string INTEGER = "INTEGER";
const std::string TEXT = "TEXT";
const std::string BLOB = "BLOB";

// SQLite constraints
const std::string PRIMARY_KEY = "PRIMARY KEY";
const std::string NOT_NULL = ""; //"NOT NULL"; TODO: SQLite doesn't support adding NOT NULL columns
const std::string DEFAULT_NULL = "DEFAULT NULL";
const std::string DEFAULT_0 = "DEFAULT 0";

namespace v1 {

const auto VERSION = 1;

const table tracks_table = {
   "tracks",
   {
      {"id", INTEGER, PRIMARY_KEY},
      {"id_artist", INTEGER},
      {"id_album", INTEGER},
      {"name", TEXT, NOT_NULL},
      {"track", INTEGER},
      {"file", TEXT, NOT_NULL}
   }
};

const table artists_table = {
   "artists",
   {
      {"id", INTEGER, PRIMARY_KEY},
      {"name", TEXT, NOT_NULL}
   }
};

const table albums_table = {
   "albums",
   {
      {"id", INTEGER, PRIMARY_KEY},
      {"id_artist", INTEGER},
      {"name", TEXT, NOT_NULL},
      {"genre", TEXT, DEFAULT_NULL},
      {"year", INTEGER, DEFAULT_0}
   }
};

const std::vector<const table*> tables = {&tracks_table, &artists_table, &albums_table};

} // v1

inline namespace v2 {

const auto VERSION = 2;

const table tracks_table = {
   v1::tracks_table.name,
   {
      v1::tracks_table.fields[0],
      v1::tracks_table.fields[1],
      v1::tracks_table.fields[2],
      v1::tracks_table.fields[3],
      v1::tracks_table.fields[4],
      v1::tracks_table.fields[5],
      {"artwork", BLOB}
   }
};

const table settings_table = {
   "settings",
   {
      {"key", TEXT, PRIMARY_KEY},
      {"value", TEXT, DEFAULT_NULL},
   }
};

using v1::artists_table;
using v1::albums_table;

const table version_table = {
   "version",
   {
      {"version", INTEGER}
   }
};

const std::vector<const table*> tables = {&tracks_table, &artists_table, &albums_table, &settings_table, &version_table};


} // v2

const auto CURRENT_VERSION = VERSION;

}}

#endif
