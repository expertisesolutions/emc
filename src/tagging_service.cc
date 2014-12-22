#include "tagging_service.hh"

#include "logger.hh"
#include "tag_consumer.hh"

#include <Ecore.hh>

#include <memory>

namespace {
   auto MAX_FILES_QUEUE_COUNT = 100;
   auto MAX_READ_TAGS_QUEUE_COUNT = 5;
}

namespace emc {

tagging_service::tagging_service(::emc::database &database)
   : database(database)
   , database_map(database)
   , files(MAX_FILES_QUEUE_COUNT)
   , tags(MAX_READ_TAGS_QUEUE_COUNT)
   , scanner(files)
   , reader(files, tags)
{}

tagging_service::~tagging_service()
{
   tags.close();
   files.close();
}

void
tagging_service::start()
{
   DBG << "Starting file scanner...";
   scanner.start();

   DBG << "Mapping database...";
   database_map.async_map(std::bind(&tagging_service::on_database_mapped, this));
}

void
tagging_service::on_database_mapped()
{
   DBG << "Starting one tag consumer...";
   consumer = std::make_shared<tag_consumer>(database, database_map, tags);
}

}
