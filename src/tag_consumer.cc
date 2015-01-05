#include "tag_consumer.hh"

#include "logger.hh"

#include <Ecore.hh>

namespace emc {

tag_consumer::tag_consumer(database &db, database_map &db_map, bounded_buffer<tag> &tags)
   : tags(tags)
   , terminated(false)
   , updating_tag(false)
   , updater(db, db_map, std::bind(&tag_consumer::on_tag_updated, this, std::placeholders::_1))
   , worker(std::bind(&tag_consumer::process, this))
{}

tag_consumer::~tag_consumer()
{}

void
tag_consumer::stop()
{
   tags.close();
   terminated = true;
   tag_updated.notify_one();
   worker.join();
}

void
tag_consumer::process()
{
   ::emc::tag tag;
   while (!terminated && tags.pop_front(tag))
     {
        DBG << "Consuming one tag...";
        efl::eina::unique_lock<efl::eina::mutex> lock(updating_tag_mutex);
        updating_tag = true;
        efl::ecore::main_loop_thread_safe_call_async(std::bind(&tag_consumer::update_tag, shared_from_this(), std::ref(tag)));
        tag_updated.wait(lock, [this]{ return terminated || !updating_tag; });
     }
}

void
tag_consumer::update_tag(tag &tag)
{
   if (terminated)
     return;

   DBG << "Resizing artwork...";
   tag.artwork = resizer.resize(tag.artwork, 128, 128);

   DBG << "Processing tag...";
   updater.update(tag);
}

void
tag_consumer::on_tag_updated(const tag &tag)
{
   if (terminated)
     return;

   {
      efl::eina::unique_lock<efl::eina::mutex> lock(updating_tag_mutex);
      updating_tag = false;
   }

   DBG << "Tag processed.";
   tag_updated.notify_one();
}

}
