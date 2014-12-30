#include "tag_reader.hh"

#include "logger.hh"

#include <attachedpictureframe.h>
#include <fileref.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <tag.h>

#include <Ecore.hh>
#include <Ecore_File.h>

#include <cctype>
#include <fstream>

namespace {

std::string
extract_file_extension(const std::string &path)
{
   const auto pos = path.rfind(".");
   if (std::string::npos == pos)
     return "";

   auto ext = path.substr(pos + 1);
   for (auto &c : ext) c = ::toupper(c);

   return ext;
}

}

namespace emc {

tag_reader::tag_reader(bounded_buffer<std::string> &files, bounded_buffer<tag> &tags)
   : files(files)
   , tags(tags)
   , worker(&tag_reader::process, this)
{}

tag_reader::~tag_reader()
{
   files.close();
   tags.close();
   worker.join();
}

void
tag_reader::process()
{
   std::string file;
   while (files.pop_front(file))
     process_file(file);
}

void
tag_reader::process_file(const std::string &path)
{
   // TODO: ignore if its already read

   TagLib::FileRef file(path.c_str());
   if (file.isNull() || !file.tag())
     return;

   auto to_string = [](const TagLib::String &str) -> std::string
     {
        if (str == TagLib::String::null)
          return "";

        const auto UNICODE = true;
        return str.to8Bit(UNICODE);
     };

   ::emc::tag new_tag;

   auto tag = file.tag();
   new_tag.file = path;
   new_tag.title = to_string(tag->title());
   new_tag.track = tag->track();
   new_tag.artist = to_string(tag->artist());
   new_tag.album = to_string(tag->album());
   new_tag.genre = to_string(tag->genre());
   new_tag.year = tag->year();

   auto audio = file.audioProperties();
   new_tag.length = audio->length();
   new_tag.bitrate = audio->bitrate();
   new_tag.samplerate = audio->sampleRate();
   new_tag.channels = audio->channels();

   const auto ext = extract_file_extension(path);

   if ("MP3" == ext)
     process_mp3(static_cast<TagLib::MPEG::File*>(file.file()), new_tag);

   tags.push_back(std::move(new_tag));
}

void
tag_reader::process_mp3(TagLib::MPEG::File *file, tag& new_tag)
{
   auto artwork = get_mp3_artwork(file);

   efl::ecore::main_loop_thread_safe_call_sync([this, &artwork]() mutable
     {
        artwork = resizer.resize(artwork, 128, 128);
     });

   new_tag.artwork = move(artwork);
}

std::vector<unsigned char>
tag_reader::get_mp3_artwork(TagLib::MPEG::File *file)
{
   auto tag = file->ID3v2Tag();
   if (nullptr == tag)
     return std::vector<unsigned char>();

   using TagLib::ID3v2::AttachedPictureFrame;

   auto apic_frames = tag->frameListMap()["APIC"];
   auto pic_frames = tag->frameListMap()["PIC"];

   auto find_apic_frame = [&apic_frames, &pic_frames](AttachedPictureFrame::Type type) -> AttachedPictureFrame*
     {
        auto find = [type](TagLib::ID3v2::FrameList &frames) -> AttachedPictureFrame*
          {
             for (auto &frame : frames)
               {
                  auto apic_frame = static_cast<AttachedPictureFrame*>(frame);
                  if (apic_frame->type() == type)
                    return apic_frame;
               }
             return nullptr;
          };

       auto result = find(apic_frames);
       if (!result)
         result = find(pic_frames);
       return result;
     };

   auto artwork_frame = find_apic_frame(AttachedPictureFrame::Media);
   if (!artwork_frame || artwork_frame->picture().isEmpty())
     artwork_frame = find_apic_frame(AttachedPictureFrame::FrontCover);
   if (!artwork_frame || artwork_frame->picture().isEmpty())
     artwork_frame = find_apic_frame(AttachedPictureFrame::Other);
   if (!artwork_frame || artwork_frame->picture().isEmpty())
     return std::vector<unsigned char>();

   auto picture = artwork_frame->picture();
   std::vector<unsigned char> artwork;
   artwork.reserve(picture.size());
   std::copy(std::begin(picture), std::end(picture), back_inserter(artwork));
   return artwork;
}

}
