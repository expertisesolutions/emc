#ifndef _TAG_CONSUMER_HH
#define _TAG_CONSUMER_HH

#include "bounded_buffer.hh"
#include "picture_resizer.hh"
#include "tag.hh"
#include "tag_updater.hh"

namespace emc {

class database;
class database_map;

/**
 * Consumes tags and updates database information based on them
 */
class tag_consumer : public std::enable_shared_from_this<tag_consumer>
{
public:
   tag_consumer(database &db, database_map &db_map, bounded_buffer<tag> &tags);
   ~tag_consumer();

private:
   void process();
   void update_tag(tag &tag);
   void on_tag_updated(const tag &tag);

private:
   bounded_buffer<tag> &tags;
   picture_resizer resizer;
   tag_updater updater;
   volatile bool terminated;
   volatile bool updating_tag;
   ::efl::eina::condition_variable tag_updated;
   ::efl::eina::mutex updating_tag_mutex;
   ::efl::eina::thread worker;
};

}

#endif
