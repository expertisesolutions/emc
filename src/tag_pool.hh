#ifndef _TAG_POOL_HH
#define _TAG_POOL_HH

#include "tag.hh"

#include <Eina.hh>

#include <unordered_map>

namespace emc
{

class tag;

class tag_pool
{
public:
   tag_pool(size_t size);
   ~tag_pool();

   void add(const tag &tag);
   void remove(const tag &tag);

private:
   size_t size;
   bool terminated;
   ::efl::eina::condition_variable pool_full;
   ::efl::eina::mutex pool_mutex;
   std::unordered_map<std::string, tag> pool;
};

}

#endif