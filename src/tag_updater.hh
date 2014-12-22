#ifndef _TAG_UPDATER_HH
#define _TAG_UPDATER_HH

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

namespace emc {

class database;
class tag;
class database_map;
class tag_processor;

/**
 * Updates new read tags to database
 */
class tag_updater
{
public:
   tag_updater(database &db, database_map &db_map, std::function<void(const tag &tag)> handler);
   ~tag_updater();

   void update(const tag &tag);
   bool is_updating() const;

private:
   void process();
   void next_processor(const tag &tag);

private:
   database &db;
   database_map &db_map;
   std::function<void(const tag &tag)> handler;
   std::queue<std::unique_ptr<tag_processor>> processing_tags;
};

}

#endif
