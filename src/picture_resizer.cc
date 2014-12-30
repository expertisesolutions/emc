#include "picture_resizer.hh"

#include "logger.hh"

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>

extern "C" {
#include <jpeglib.h>
}
#include <setjmp.h>

// Adapted functions from eet_image.c. Evas doesn't support saving to stream yet.
namespace  {

   struct _JPEG_error_mgr
   {
      struct jpeg_error_mgr pub;
      jmp_buf setjmp_buffer;
   };

   typedef struct _JPEG_error_mgr *emptr;

   void
   jpeg_error_exit_cb(j_common_ptr cinfo)
   {
      char buffer[JMSG_LENGTH_MAX];
      emptr errmgr;

      (*cinfo->err->format_message)(cinfo, buffer);
      ERR << buffer;
      errmgr = (emptr)cinfo->err;
      longjmp(errmgr->setjmp_buffer, 1);
   }

   void
   jpeg_output_message_cb(j_common_ptr cinfo)
   {
      char buffer[JMSG_LENGTH_MAX];
      (*cinfo->err->format_message)(cinfo, buffer);
      ERR << buffer;
   }

   void
   jpeg_emit_message_cb(j_common_ptr cinfo, int msg_level)
   {
      char buffer[JMSG_LENGTH_MAX];
      struct jpeg_error_mgr *err;

      err = cinfo->err;
      if (msg_level < 0)
        {
           if ((err->num_warnings == 0) || (err->trace_level >= 3))
             {
                (*cinfo->err->format_message)(cinfo, buffer);
                WRN << buffer;
             }
           err->num_warnings++;
        }
      else
        {
           if (err->trace_level >= msg_level)
             {
                (*cinfo->err->format_message)(cinfo, buffer);
                INF << buffer;
             }
        }
   }

   unsigned char *
   jpeg_encode(int *size, const void *data, size_t w, size_t h, int quality)
   {
      struct jpeg_compress_struct cinfo = {};
      struct _JPEG_error_mgr jerr = {};

      cinfo.err = jpeg_std_error(&(jerr.pub));
      jerr.pub.error_exit = jpeg_error_exit_cb;
      jerr.pub.emit_message = jpeg_emit_message_cb;
      jerr.pub.output_message = jpeg_output_message_cb;
      if (setjmp(jerr.setjmp_buffer))
        return nullptr;

      jpeg_create_compress(&cinfo);

      unsigned char *outbuffer = nullptr;
      size_t outsize = 0;
      jpeg_mem_dest(&cinfo, &outbuffer, &outsize);

      cinfo.image_width = w;
      cinfo.image_height = h;
      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB;
      cinfo.optimize_coding = FALSE;
      cinfo.dct_method = JDCT_ISLOW;
      if (quality < 60) cinfo.dct_method = JDCT_IFAST;
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, quality, TRUE);

      if (quality >= 90)
        {
           cinfo.comp_info[0].h_samp_factor = 1;
           cinfo.comp_info[0].v_samp_factor = 1;
           cinfo.comp_info[1].h_samp_factor = 1;
           cinfo.comp_info[1].v_samp_factor = 1;
           cinfo.comp_info[2].h_samp_factor = 1;
           cinfo.comp_info[2].v_samp_factor = 1;
        }

      jpeg_start_compress(&cinfo, TRUE);

      unsigned char buf[3 * w];
      while (cinfo.next_scanline < cinfo.image_height)
        {
           /* convert scaline from ARGB to RGB packed */
           auto ptr = static_cast<const unsigned int*>(data) + cinfo.next_scanline * w;
           for (size_t j = 0, i = 0; i < w; i++)
             {
                buf[j++] = ((*ptr) >> 16) & 0xff;
                buf[j++] = ((*ptr) >> 8) & 0xff;
                buf[j++] = ((*ptr)) & 0xff;
                ptr++;
             }

           auto row = &buf[0];
           auto jbuf = static_cast<JSAMPARRAY>(&row);
           jpeg_write_scanlines(&cinfo, jbuf, 1);
        }

      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);

      *size = outsize;
      return outbuffer;
   }

}

namespace emc {

picture_resizer::picture_resizer()
   : ee(nullptr)
   , e(nullptr)
   , o(nullptr)
   , sub_ee(nullptr)
   , sub_e(nullptr)
   , img(nullptr)
{

   if (!evas_init() || !ecore_init() || !ecore_evas_init())
     {
        ERR << "Could not initialize Evas/Ecore/Ecore_Evas";
        return;
     }

   ee = ecore_evas_buffer_new(1, 1);
   e = ecore_evas_get(ee);
   if (!e)
     {
        ERR << "could not create ecore evas buffer";
        return;
     }

   evas_image_cache_set(e, 0);
   evas_font_cache_set(e, 0);

   o = ecore_evas_object_image_new(ee);
   if (!o)
     {
        ERR << "could not create sub ecore evas buffer";
        return;
     }

   sub_ee = ecore_evas_object_ecore_evas_get(o);
   sub_e = ecore_evas_object_evas_get(o);
   ecore_evas_alpha_set(sub_ee, EINA_TRUE);

   evas_image_cache_set(sub_e, 0);
   evas_font_cache_set(sub_e, 0);

   img = evas_object_image_add(sub_e);
   if (!img)
     {
        ERR << "could not create source objects.";
        return;
     }
}

picture_resizer::~picture_resizer()
{
   DBG << "Freeing Evas Object";
   if (o) evas_object_del(o);
   DBG << "Freeing Evas Buffer";
   if (ee) ecore_evas_free(ee);

   ecore_evas_shutdown();
   ecore_shutdown();
   evas_shutdown();
}

std::vector<unsigned char>
picture_resizer::resize(const std::vector<unsigned char> &buffer, size_t w, size_t h)
{
   DBG << "Resizing";
   reset(w, h);

   if (!load(buffer))
     return std::vector<unsigned char>();

   std::tie(w, h) = calculate_aspect(w, h);
   if (0 == w || h == 0)
     return std::vector<unsigned char>();

   resize(w, h);

   render(w, h);

   return encode(w, h);
}

void
picture_resizer::reset(size_t w, size_t h)
{
   evas_object_hide(img);
   evas_object_image_file_set(img, nullptr, nullptr);
   evas_object_image_load_size_set(img, w, h);
}

bool
picture_resizer::load(const std::vector<unsigned char> &buffer)
{
   evas_object_image_load_orientation_set(img, EINA_TRUE);
   evas_object_image_memfile_set(img,
                                 const_cast<unsigned char*>(buffer.data()),
                                 buffer.size(),
                                 nullptr,
                                 nullptr);
   evas_object_show(img);

   auto error = evas_object_image_load_error_get(img);
   if (EVAS_LOAD_ERROR_NONE != error)
     {
        ERR << "Could not load image: " << error;
        return false;
     }

   return true;
}

std::pair<size_t, size_t>
picture_resizer::calculate_aspect(size_t w, size_t h)
{
   Evas_Coord width, height;
   evas_object_image_size_get(img, &width, &height);
   if ((width <= 0) || (height <= 0))
     {
        ERR << "Invalid image dimensions: " << width << " x " << height;
        return std::make_pair(0, 0);
     }

   auto ratio = width / (double)height;
   if (height > width)
     w = w * ratio;
   else
     h = h / ratio;

   return std::make_pair(w, h);
}

void
picture_resizer::resize(size_t w, size_t h)
{
   evas_object_move(img, 0, 0);
   evas_object_resize(img, w, h);
   evas_object_image_fill_set(img, 0, 0, w, h);
   evas_object_image_size_set(o, w, h);
   ecore_evas_resize(sub_ee, w, h);
}

void
picture_resizer::render(size_t w, size_t h)
{
   evas_damage_rectangle_add(sub_e, 0, 0, w, h);
   evas_render(sub_e);
}

std::vector<unsigned char>
picture_resizer::encode(size_t w, size_t h)
{
   const void *pixels = ecore_evas_buffer_pixels_get(sub_ee);

   int size = 0;
   auto data = jpeg_encode(&size, pixels, w, h, 80);
   if (!data)
     {
        ERR << "Error: No data";
        return std::vector<unsigned char>();
     }

   std::vector<unsigned char> jpeg_stream(data, data + size);
   free(data);

   DBG << "Returning jpeg stream";
   return jpeg_stream;
}

}
