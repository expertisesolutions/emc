#ifndef _PICTURE_RESIZER_HH
#define _PICTURE_RESIZER_HH

#include <Evas.h>
#include <Ecore_Evas.h>

#include <utility>
#include <vector>

namespace emc {

class picture_resizer
{
public:
   picture_resizer();
   ~picture_resizer();

   /**
    * Resizes a image to jpeg keeping aspect ratio
    *
    * @param buffer The image buffer (supported evas image)
    * @param w The new width
    * @param h The new height
    * @return Returns the resized jpeg image buffer
    * @attention Must be called from main loop
    */
   std::vector<unsigned char> resize(const std::vector<unsigned char> &buffer, size_t w, size_t h);

private:
   void reset(size_t w, size_t h);
   bool load(const std::vector<unsigned char> &buffer);
   void resize(size_t w, size_t h);
   std::pair<size_t, size_t> calculate_aspect(size_t w, size_t h);
   void render(size_t w, size_t h);
   std::vector<unsigned char> encode(size_t w, size_t h);

private:
   Ecore_Evas *ee;
   Evas *e;
   Evas_Object *o;
   Ecore_Evas *sub_ee;
   Evas *sub_e;
   Evas_Object *img;
};


}

#endif
