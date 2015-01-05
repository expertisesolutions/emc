#include "tagging_context.hh"

#include "tag_consumer.hh"

#include <memory>

namespace emc {

tagging_context::tagging_context(::emc::database &database,
                                 ::emc::database_map &database_map,
                                 size_t max_files,
                                 size_t max_tags,
                                 const std::string &path)
   : files(max_files)
   , tags(max_tags)
   , scanner(files)
   , reader(files, tags)
   , consumer(std::make_shared<tag_consumer>(database, database_map, tags))
   , path(path)
{}

tagging_context::~tagging_context()
{
   stop();
}

void
tagging_context::stop()
{
   consumer->stop();
   tags.close();
   files.close();
}

void
tagging_context::start()
{
   scanner.scan(path);
}

}
