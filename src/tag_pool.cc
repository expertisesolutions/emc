#include "tag_pool.hh"

#include "logger.hh"

#include <cassert>

namespace emc
{

tag_pool::tag_pool(size_t size)
   : size(size)
   , terminated(false)
{}

tag_pool::~tag_pool()
{
   terminated = true;
   pool_full.notify_one();
   // TODO: Wait for
}

void
tag_pool::add(const tag &tag)
{
   if (terminated) return;

   {
      efl::eina::unique_lock<efl::eina::mutex> lock(pool_mutex);
      if (pool.size() == size)
        {
           pool_full.wait(lock);
           if (terminated) return;
        }
      pool.insert(std::make_pair(tag.file, tag));
   }
}

void
tag_pool::remove(const tag &tag)
{
   {
      efl::eina::unique_lock<efl::eina::mutex> lock(pool_mutex);
      auto it = pool.find(tag.file);
      assert(end(pool) != it);
      pool.erase(it);
   }
   pool_full.notify_one();
}

}
