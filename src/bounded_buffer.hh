#ifndef _BOUNDED_BUFFER_HH
#define _BOUNDED_BUFFER_HH

#include <Eina.hh>

#include <cassert>
#include <queue>

namespace emc {

/**
 * A FIFO container for multi-thread access and maximum capacity to lock working
 * threads.
 * @see Producer-Consumer problem.
 */
template<class T>
class bounded_buffer
{
public:
   bounded_buffer(size_t capacity)
      : capacity(capacity)
      , closed(false)
   {}

   bounded_buffer(const bounded_buffer&) = delete;
   bounded_buffer& operator=(const bounded_buffer&) = delete;

   ~bounded_buffer()
   {
      assert(closed && "The buffer must be closed");
   }

   void push_back(const T &value)
   {
      auto temp(value);
      push(move(temp));
   }

   void push_back(T &&value)
   {
      {
         efl::eina::unique_lock<efl::eina::mutex> lock(buffer_mutex);
         new_slot.wait(buffer_mutex, [this]{ return closed || buffer.size() < capacity; });
         if (closed) return;

         buffer.push(std::move(value));
      }

      new_item.notify_one();
   }

   bool pop_front(T &value)
   {
      {
         efl::eina::unique_lock<efl::eina::mutex> lock(buffer_mutex);
         new_item.wait(buffer_mutex, [this]{ return closed || !buffer.empty(); });
         if (closed) return false;

         value = std::move(buffer.front());
         buffer.pop();
      }

      new_slot.notify_one();
      return true;
   }

   void close()
   {
      closed = true;
      new_slot.notify_all();
      new_item.notify_all();
   }

   bool is_closed() const
   {
      return closed;
   }

private:
   size_t capacity;
   volatile bool closed;
   ::efl::eina::condition_variable new_slot;
   ::efl::eina::condition_variable new_item;
   ::efl::eina::mutex buffer_mutex;
   std::queue<T> buffer;
};

}

#endif
