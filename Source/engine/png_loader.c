#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "png_loader.h"
#include "util.h"

#ifndef TARGET_OS_IPHONE
    #include <png.h>
#else
    #include <CoreGraphics/CoreGraphics.h>
#endif

static void png_destroy(Png_t *self);

Png_t *png_load(const char *aPath) {
    Png_t *self = obj_create_autoreleased(sizeof(Png_t), (Obj_destructor_t)&png_destroy);
#ifndef TARGET_OS_EMBEDDED
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    int color_type;
    FILE *fp;

    if ((fp = fopen(aPath, "rb")) == NULL)
        return false;

    /* Create and initialize the png_struct
     * with the desired error handler
     * functions.  If you want to use the
     * default stderr and longjump method,
     * you can supply NULL for the last
     * three parameters.  We also supply the
     * the compiler header file version, so
     * that we know if the application
     * was compiled with a compatible version
     * of the library.  REQUIRED
     */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL) {
        fclose(fp);
        return false;
    }

    /* Allocate/initialize the memory
     * for image information.  REQUIRED. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return false;
    }

    /* Set error handling if you are
     * using the setjmp/longjmp method
     * (this is the normal method of
     * doing things with libpng).
     * REQUIRED unless you  set up
     * your own error handlers in
     * the png_create_read_struct()
     * earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated
         * with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(fp);
        /* If we get here, we had a
         * problem reading the file */
        return false;
    }

    /* Set up the output control if
     * you are using standard C streams */
    png_init_io(png_ptr, fp);

    /* If we have already
     * read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);

    /*
     * If you have enough memory to read
     * in the entire image at once, and
     * you need to specify only
     * transforms that can be controlled
     * with one of the PNG_TRANSFORM_*
     * bits (this presently excludes
     * dithering, filling, setting
     * background, and doing gamma
     * adjustment), then you can read the
     * entire image (including pixels)
     * into the info structure with this
     * call
     *
     * PNG_TRANSFORM_STRIP_16 |
     * PNG_TRANSFORM_PACKING  forces 8 bit
     * PNG_TRANSFORM_EXPAND forces to
     *  expand a palette into RGB
     */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, (png_voidp)NULL);
    self->width = png_get_image_width(png_ptr, info_ptr);
    self->height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
    switch (color_type) {
        case PNG_COLOR_TYPE_RGBA:
            self->hasAlpha = true;
            break;
        case PNG_COLOR_TYPE_RGB:
            self->hasAlpha = false;
            break;
        default:
			debug_log("Color type: %d not supported", color_type);
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return false;
    }
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    self->data = (unsigned char*) malloc(row_bytes * (self->height));

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (int i = 0; i < self->height; i++) {
        // note that png is ordered top to
        // bottom, but OpenGL expect it bottom to top
        // so the order or swapped
        memcpy((void*)(self->data+(row_bytes * i)), row_pointers[i], row_bytes);
    }

    /* Clean up after the read,
     * and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

    /* Close the file */
    fclose(fp);
#else
    // We're on iOS, so let's use CG
    CGDataProviderRef provider = CGDataProviderCreateWithFilename(aPath);
    if(!provider)
        return false;
    CGImageRef cgImg = CGImageCreateWithPNGDataProvider(provider,
                                                        NULL,
                                                        false,
                                                        kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);
    if(!cgImg)
        return false;

    self->width    = CGImageGetWidth(cgImg);
    self->height   = CGImageGetHeight(cgImg);
    self->hasAlpha = CGImageGetAlphaInfo(cgImg) != kCGImageAlphaNone;
    self->cfData   = CGDataProviderCopyData(CGImageGetDataProvider(cgImg));
    self->data     = CFDataGetBytePtr(self->cfData);
#endif
    return self;
}

void png_destroy(Png_t *self)
{
#ifdef TARGET_OS_EMBEDDED
    CFRelease(self->cfData);
#else
    free((void*)self->data);
#endif
}
