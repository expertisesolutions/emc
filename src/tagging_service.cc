#include "tagging_service.hh"

#include "logger.hh"
#include "tag_consumer.hh"
#include "tagging_context.hh"

namespace {
   auto MAX_FILES_QUEUE_COUNT = 100;
   auto MAX_READ_TAGS_QUEUE_COUNT = 5;
}

namespace emc {

tagging_service::tagging_service(::emc::database &database,
                                 ::emc::database_map &database_map)
   : database(database)
   , database_map(database_map)
{}

tagging_service::~tagging_service()
{
   if (context)
     context->stop();
}

void
tagging_service::scan(const std::string &path)
{
   DBG << "Scanning for media files in: " << path;
   create_context(path);
   context->start();
}

void
tagging_service::create_context(const std::string &path)
{
   context.reset(new tagging_context(database,
                                     database_map,
                                     MAX_FILES_QUEUE_COUNT,
                                     MAX_READ_TAGS_QUEUE_COUNT,
                                     path));
}

}
