#include "tag_updater.hh"

#include "database.hh"
#include "database_map.hh"
#include "emodel_helpers.hh"
#include "logger.hh"
#include "tag.hh"
#include "tag_processor.hh"

namespace {
   template<typename T, typename... Args>
   std::unique_ptr<T> make_unique(Args&&... args)
   {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
   }
}

namespace emc {

tag_updater::tag_updater(database &db, database_map &db_map, std::function<void(const tag &tag)> handler)
   : db(db)
   , db_map(db_map)
   , handler(handler)
{}

tag_updater::~tag_updater()
{}

void
tag_updater::update(const tag &tag)
{
   DBG << "Processing tag: " << tag.file;
   auto &artists = db.artists_get();
   auto &albums = db.albums_get();
   auto &tracks = db.tracks_get();

   auto next_processor = std::bind(&tag_updater::next_processor, this, std::cref(tag));

   auto artist_processor = make_unique<tag_processor>(db_map.artists_map_get(), artists, tag.artist, next_processor,
     [&tag](esql::model_row row)
     {
        DBG << "Setting artist properties: " << tag.artist;
        emc::emodel_helpers::property_set(row, "name", tag.artist);
     });

   auto album_processor = make_unique<tag_processor>(db_map.albums_map_get(), albums, tag.album, next_processor,
     [&tag, this](esql::model_row row)
     {
        DBG << "Setting album properties: " << tag.album;
        auto artist_id = db_map.artist_id_get(tag.artist);
        emc::emodel_helpers::property_set(row, "name", tag.album);
        if (database::INVALID_ID != artist_id)
          emc::emodel_helpers::property_set(row, "id_artist", artist_id);
        emc::emodel_helpers::property_set(row, "genre", tag.genre);
        emc::emodel_helpers::property_set(row, "year", tag.year);
     });

   auto track_processor = make_unique<tag_processor>(db_map.tracks_map_get(), tracks, tag.file, next_processor,
     [&tag, this](esql::model_row row)
     {
        DBG << "Setting track properties: " << tag.file;
        auto artist_id = db_map.artist_id_get(tag.artist);
        auto album_id = db_map.album_id_get(tag.album);
        emc::emodel_helpers::property_set(row, "file", tag.file);
        if (database::INVALID_ID != artist_id)
          emc::emodel_helpers::property_set(row, "id_artist", artist_id);
        if (database::INVALID_ID != album_id)
          emc::emodel_helpers::property_set(row, "id_album", album_id);
        emc::emodel_helpers::property_set(row, "name", tag.title);
        emc::emodel_helpers::property_set(row, "track", tag.track);
        emc::emodel_helpers::property_set(row, "artwork", tag.artwork);
        emc::emodel_helpers::property_set(row, "length", tag.length);
        emc::emodel_helpers::property_set(row, "bitrate", tag.bitrate);
        emc::emodel_helpers::property_set(row, "samplerate", tag.samplerate);
        emc::emodel_helpers::property_set(row, "channels", tag.channels);
     });

   processing_tags.push(move(artist_processor));
   processing_tags.push(move(album_processor));
   processing_tags.push(move(track_processor));

   process();

   if (!is_updating())
     handler(tag);
}

void
tag_updater::process()
{
   DBG << "Processing " << processing_tags.size();
   while (!processing_tags.empty())
     {
        bool is_processing_pending = processing_tags.front()->process();
        if (is_processing_pending) return;

        DBG << "Going to next processor";
        processing_tags.pop();
     }
}

void
tag_updater::next_processor(const tag &tag)
{
   if (!processing_tags.empty())
     {
        DBG << "Going to next processor";
        processing_tags.pop();
     }

   process();

   if (!is_updating())
     handler(tag);
}

bool
tag_updater::is_updating() const
{
   return !processing_tags.empty();
}

}
