/*****************************************************************************
 * kvb image reader
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 14:49:17 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1465 $
 * Created: 2009.07.19
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtGui>
#include <jpeglib.h>
#include <png.h>
#include "kbvImageReader.h"


KbvImageReader::KbvImageReader() : QImageReader()
{
}

KbvImageReader::~KbvImageReader()
{
  //qDebug() << "KbvImageReader::~KbvImageReader"; //###########
}

/*************************************************************************//*!
 * Detects the quality of jpeg image.
 * If the jpeg was created using a straight scaling of the standard image
 * quantization tables and the 0-100 quality was used based on the Independent
 * JPEG Group's formula, then, then the original value can be obtained from
 * luminance table.
 */
int    KbvImageReader::quality()
  {
    JQUANT_TBL  **qtblptr;
    struct      jpeg_decompress_struct cinfo;
    struct      jpeg_error_mgr jerr;
    int         coeff54=0, qual=0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);


    FILE   *imgfile;
    char   *name;
    QByteArray arr = fileName().toLatin1();
    name = arr.data();


    if (((imgfile = fopen(name, "rb")) == nullptr) | (format() != "jpeg"))
      {
        jpeg_destroy_decompress(&cinfo);
        return 90;
      }
    jpeg_stdio_src(&cinfo, imgfile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    //0 = luminance compression table, standard luminance coefficient[54] = 120
    qtblptr = &cinfo.quant_tbl_ptrs[0];
    if (*qtblptr != nullptr)
      {
        coeff54 = (*qtblptr)->quantval[54];
        if (coeff54 <= Kbv::StdLumCoeff54)
          {
            qual = 100 - 50 * coeff54 / Kbv::StdLumCoeff54;
          }
        else
          {
            qual = 50 * Kbv::StdLumCoeff54 / coeff54;
            if (qual < 10)
              {
                qual = 10;
              }
          }
      }
    fclose(imgfile);
    jpeg_destroy_decompress(&cinfo);
    return qual;
  }

/*************************************************************************//*!
 * Detects the compression of tiff image.
 */
int    KbvImageReader::compression()
  {
    return      1;
  }

/*************************************************************************//*!
 * Detects the gamma value of png image. The gamma is read as reciprocal
 * value.
 */
float    KbvImageReader::gamma()
  {
    FILE   *imgfile;
    char   *name;
    double gamma;
    png_structp png_ptr;
    png_infop   info_ptr;
    png_infop   end_info;

    QByteArray arr = fileName().toLatin1();
    name = arr.data();

    if (((imgfile = fopen(name, "rb")) !=nullptr))
      {
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png_ptr)
          {
            info_ptr = png_create_info_struct(png_ptr);
            if (info_ptr)
              {
                end_info = png_create_info_struct(png_ptr);
                if (end_info)
                  {
                    png_init_io(png_ptr, imgfile);
                    png_read_info(png_ptr, info_ptr);
                    png_get_gAMA(png_ptr, info_ptr, &gamma);
                    //Default, if no gamma chunk could be found (gamma=0)
                    if (gamma < double(0.9*Kbv::pngGammaScale/Kbv::pngGammaMax))
                      {
                        gamma = double(Kbv::pngGammaScale/Kbv::pngGammaStd);
                      }
                    //Free allocated memory
                    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
                    fclose(imgfile);
                    //qDebug() <<"kbvImageReader::gamma" << 1.0/gamma; //###########
                    return      float(1.0/gamma);
                  }
              }
            //Read_struct but no info_struct
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
          }
        fclose(imgfile);
      }
    //file not opened
    return float(Kbv::pngGammaStd/Kbv::pngGammaScale);
  }

/****************************************************************************/
