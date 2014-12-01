#ifndef _DB_SCHEMA_HH
#define _DB_SCHEMA_HH

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

// SQLite constraints
const std::string PRIMARY_KEY = "PRIMARY KEY";
const std::string NOT_NULL = ""; //"NOT NULL"; TODO: SQLite doesn't support adding NOT NULL columns
const std::string DEFAULT_NULL = "DEFAULT NULL";
const std::string DEFAULT_0 = "DEFAULT 0";

inline namespace v1 {

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


void create_database(esql::model &database);

}}

#endif
