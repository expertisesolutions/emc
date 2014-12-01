#include "file_scanner.hh"

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <ostream>

#include <fileref.h>
#include <tag.h>

namespace {
   template<typename T, typename... Args>
   std::unique_ptr<T> make_unique(Args&&... args)
   {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
   }

   template<class T>
   bool get_property(const ::emodel &model, const std::string &property, T &value)
   {
      ::efl::eina::value property_value;
      if (!model.property_get(property, property_value.native_handle()))
        {
           std::cout << "Error trying to get " << property << " property" << std::endl;
           return false;
        }

      value = ::efl::eina::get<T>(property_value);
      return true;
   }
}

namespace emc {

file_scanner::file_scanner()
{}

file_scanner::~file_scanner()
{}

void file_scanner::start()
{
   const auto paths = get_configured_paths();
   for (auto &path : paths)
     scan_path(path);
}

std::vector<std::string> file_scanner::get_configured_paths() const
{
   // TODO: The path must be configurable
   const char *home_dir = getenv("HOME");
   if (!home_dir)
     home_dir = getenv("USERPROFILE");

   std::cout << "Home directory: " << home_dir << std::endl;

   std::string music_dir = std::string(home_dir) + "/Music";

   return std::vector<std::string>(1, music_dir);
}

void file_scanner::scan_path(const std::string &path)
{
   std::cout << "Scanning path: " << path << std::endl;
   std::unique_ptr<eio::model> file_model(new eio::model());
   file_model->path_set(path);
   file_model->callback_children_count_changed_add(
     std::bind(&file_scanner::file_found, this, *file_model, std::placeholders::_3));
   file_model->load();
   files.push_back(move(file_model));
}

bool file_scanner::file_found(eio::model &file_model, void *info)
{
   auto item_count = *static_cast<unsigned int*>(info);
   std::cout << "Number of items found: " << item_count << std::endl;
   if (0 == item_count)
     return false;

   Eina_Accessor *accessor = nullptr;
   file_model.children_slice_get(0, 0, &accessor);
   if (nullptr == accessor) return false;

   // FIXME: Use EINA-CXX
   Eo *item;
   unsigned int i = 0;
   EINA_ACCESSOR_FOREACH(accessor, i, item)
     {
        ::eio::model file(::eo_ref(item));
        file.callback_load_status_add(
          std::bind(&file_scanner::file_status, this, file, std::placeholders::_3));
        file.properties_load();
     }
   return true;
}

bool file_scanner::file_status(eio::model &file, void *info)
{
   const auto load = *static_cast<Emodel_Load*>(info);
   if (!(EMODEL_LOAD_STATUS_LOADED_PROPERTIES & load.status))
     return true;

   std::string filename;
   std::string path;
   int is_directory;
   int is_link;
   //int64_t size;

   if (!get_property(file, "filename", filename)
     || !get_property(file, "path", path)
     || !get_property(file, "is_dir", is_directory)
     || !get_property(file, "is_lnk", is_link)
     //|| !get_property(file, "size", size) // efl::eina::get<int64_t> is limited, it doesn't match equal types properly like long == int64_t on 64 bits machines
     ) return false;

   if (is_directory)
     scan_path(path);
   else
     check_media_file(path);

   return false;
}

void file_scanner::check_media_file(const std::string &path)
{
   std::cout << "Checking if filename is a recognized media type: " << path << std::endl;

   TagLib::FileRef file(path.c_str());
   if (file.isNull() || !file.tag())
     return;

   using namespace std;
   TagLib::Tag *tag = file.tag();
   cout << "title - \"" << tag->title() << "\"" << endl;
   cout << "artist - \"" << tag->artist() << "\"" << endl;
   cout << "album - \"" << tag->album() << "\"" << endl;
   cout << "year - \"" << tag->year() << "\"" << endl;
   cout << "comment - \"" << tag->comment() << "\"" << endl;
   cout << "track - \"" << tag->track() << "\"" << endl;
   cout << "genre - \"" << tag->genre() << "\"" << endl;
}

}
