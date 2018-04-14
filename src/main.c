#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <png.h>

#define _POSIX_SOURCE 1
#define PNG_BYTES_TO_CHECK 8

int prechecks(char *file_name, FILE **fp);
int check_if_png(char *file_name, FILE **fp);
void print_image(png_bytep *row_pointers, png_uint_32 width, png_uint_32 height);

int main(int argc, char **argv) {
  char *image_path = NULL;
  int c;
  int rowbytes;
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;
  png_uint_32 width;
  png_uint_32 height;
  /*int bit_depth;*/

  while ((c = getopt(argc, argv, "i:v")) != -1) {
    switch (c) {
    case 'i':
      image_path = optarg;
      break;
    case 'v':
      printf("0.0.1\n");
      return 0;
    case '?':
      printf("Usage: %s [-i image path] [-v]\n", argv[0]);
      return 1;
    default:
      break;
    }
  }
  if (image_path == NULL) {
    printf("Usage: %s [-i image path] [-v]\n", argv[0]);
    return 1;
  }

  fp = fopen(image_path, "rb");

  if (!prechecks(image_path, &fp)) {
    return 1;
  }

  /* Setup png_struct and png_info structs */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    printf("Error creating png_ptr\n");
    return 1;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr,
      (png_infopp)NULL, (png_infopp)NULL);
    printf("Error creating info_ptr\n");
    return 1;
  }

  end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr,
      (png_infopp)NULL);
  }

  /* instead of `setjmp` we could compile with PNG_SETJMP_NOT_SUPPORTED */
  /* pg. 8 http://www.libpng.org/pub/png/libpng-1.4.0-manual.pdf */
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr,
        &end_info);
    fclose(fp);
    printf("libpng encountered an error\n");
    return 1;
  }


  /* let libpng know that there are
   * some bytes missing from the start of the file.
   */
  png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
  png_init_io(png_ptr, fp);


  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  /*bit_depth = png_get_bit_depth(png_ptr, info_ptr);*/
  
  png_bytep row_pointers[height];
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  for (png_uint_32 row = 0; row < height; row++) {
    row_pointers[row] = png_malloc(png_ptr, rowbytes);
  }

  png_read_image(png_ptr, row_pointers);

  print_image(row_pointers, width, height);


  /* clean up */
  png_read_end(png_ptr, end_info);
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  return 0;
}

void print_image(png_bytep *row_pointers, png_uint_32 width, png_uint_32 height) {
  int red;
  int green;
  int blue;
  int interval = 1;
  struct winsize w;
  
  int ansi_code = 16;

  /* Get screen row/col */
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  if ( ((width*2) > w.ws_col) && ((width*2) - w.ws_col) < (height - w.ws_row)) {
    interval =  (width*2) / w.ws_col;
  } else if (height > w.ws_row) {
    interval = height / w.ws_row;
  }

  printf ("lines %d height: %d\n", w.ws_row, height);
  printf ("columns %d width: %d\n", w.ws_col, width);

  printf ("interavl %d\n", interval);


  for (png_uint_32 row = 0; row < height; row += interval) {
    /*break;*/
    for (png_uint_32 col = 0; col < (width * 4); col += (interval * 4)) {
      ansi_code = 16;

      red = (int) floor(row_pointers[row][col+0] / 42.50);
      green = (int) floor(row_pointers[row][col+1] / 42.50);
      blue = (int) floor(row_pointers[row][col+2] / 42.50);

      if (red == 6)
        red = 5;
      if (green == 6)
        green = 5;
      if (blue == 6)
        blue = 5;

      ansi_code += blue;
      ansi_code += (red * 36);
      ansi_code += (green * 6);

      printf("\x1B[48;5;%dm  ", ansi_code);
    }
    printf("\x1B[0m\n");
  }

}

int prechecks(char *file_name, FILE **fp) {
  int is_png;

  /* Can we open the file */
  if (!fp) {
    printf("Error opening file: %s\n", file_name);
    return 0;
  }

  /* Is the file a PNG? */
  is_png = check_if_png(file_name, &*fp);
  if (!is_png) {
    printf("Error file: %s, is not a PNG file\n", file_name);
    return 0;
  }

  return 1;
}

/* Most of this function is from libpng-1.5.13/example.c */
int check_if_png(char *file_name, FILE **fp){
  unsigned char header[PNG_BYTES_TO_CHECK];

  if((*fp = fopen(file_name, "rb")) == NULL) {
    return 0;
  }

  /* Read in some of the signature bytes */
  if(fread(header, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK)
    return 0;

  /* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
  Return nonzero(true) if they match */

  return(!png_sig_cmp(header, (png_size_t)0, PNG_BYTES_TO_CHECK));
}

