/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include "settingsmodel.hh"

#define VIDEODIR "~/Videos/"
#define AUDIODIR "~/Music/"
#define PICTUREDIR "~/Pictures/"

namespace emc {

settingsmodel::settingsmodel() :
        video_dir(VIDEODIR),
        audio_dir(AUDIODIR)
{
}

settingsmodel&
settingsmodel::get()
{
   static settingsmodel settings;
   return settings;
}

std::string
settingsmodel::videopath_get()
{
   return video_dir;
}

std::string
settingsmodel::audiopath_get()
{
   return audio_dir;
}

} //emc
