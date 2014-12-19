#include "row_map.hh"

namespace emc {

row_map::row_map()
{}

row_map::~row_map()
{}

void
row_map::add(const std::string &key, esql::model_row &row)
{
   map.insert(std::make_pair(key, row));
}

esql::model_row
row_map::find(const std::string &key) const
{
   auto it = map.find(key);
   if (end(map) != it)
     return it->second;

   return esql::model_row(nullptr);
}

bool row_map::exists(const std::string &key) const
{
   auto it = map.find(key);
   return end(map) != it;
}

}
