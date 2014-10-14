/**
 * EMC - Enlightenment Media Center
 *    Audio/Video Player
 */

#include "settingsmodel.hh"

#define VIDEODIR "/Videos"
#define AUDIODIR "/Music"
#define PICTUREDIR "/Pictures"

namespace emc {

settingsmodel::settingsmodel()
{
   std::cout << "IN" << std::endl;
   std::string homepath(".");
   char *tmp = getenv("HOME");
   std::cout << tmp << std::endl;

   if(tmp) homepath = std::string(tmp);

   video_dir = std::string(homepath + VIDEODIR);
   audio_dir = std::string(homepath + AUDIODIR);
   std::cout << "OUT" << std::endl;
}

std::string
settingsmodel::video_rootpath_get()
{
   std::cout << __FUNCTION__ << std::endl;
   return video_dir;
}

std::string
settingsmodel::audio_rootpath_get()
{
   std::cout << __FUNCTION__ << std::endl;
   return audio_dir;
}

} //emc
