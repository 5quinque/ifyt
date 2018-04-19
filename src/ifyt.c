#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <png.h>
#include "ifyt.h"

#define _POSIX_SOURCE 1
#define PNG_BYTES_TO_CHECK 8
#define USAGE "Usage: ifyt [image path] [-vth]"

int main(int argc, char **argv) {
  char *image_path = NULL;
  volatile int truecolor = 0;
  int c;
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;
  png_uint_32 width;
  png_uint_32 height;
  png_bytep *row_pointers = NULL;
  int color_type;

  while ((c = getopt(argc, argv, "vtha")) != -1) {
    switch (c) {
    case 't':
      truecolor = 1;
      break;
    case 'v':
      printf("version 0.0.2\n");
      return 0;
      break;
    case 'h':
      printf("%s\n", USAGE);
      return 0;
    case '?':
      printf("%s\n", USAGE);
      return 1;
    default:
      break;
    }
  }
  for (int index = optind; index < argc; index++) {
    image_path = argv[index];
  }
  if (!image_path) {
    printf("%s\n", USAGE);
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

  get_image_and_info(png_ptr, info_ptr, &row_pointers, &width, &height, &color_type);
  
  print_image(row_pointers, width, height, color_type, truecolor);

  /* clean up */
  png_read_end(png_ptr, end_info);
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  for (png_uint_32 row = 0; row < height; row++) {
    free(row_pointers[row]);
  }

  free(row_pointers);
  fclose(fp);

  return 0;
}

int get_image_and_info(png_structp png_ptr, png_infop info_ptr,
    png_bytep **row_pointers, png_uint_32 *width, png_uint_32 *height, int *color_type) {
  int rowbytes;

  png_read_info(png_ptr, info_ptr);

  *width = png_get_image_width(png_ptr, info_ptr);
  *height = png_get_image_height(png_ptr, info_ptr);
  *color_type = png_get_color_type(png_ptr, info_ptr);
  
  *row_pointers = malloc(*height * sizeof(png_bytep));
  if (!*row_pointers) {
    printf("Error allocating memory for `row_pointers`\n");
    return 0;
  }

  rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  for (png_uint_32 row = 0; row < *height; row++) {
    (*row_pointers)[row] = png_malloc(png_ptr, rowbytes);
    if (!(*row_pointers)[row]) {
      printf("Error allocating memory for `row_pointers[%d]`\n", row);
      return 0;
    }
  }

  png_read_image(png_ptr, *row_pointers);

  return 1;
}

void print_image(png_bytep *row_pointers, png_uint_32 width,
    png_uint_32 height, int color_type, int truecolor) {
  struct rgb bg;
  struct rgb fg;
  struct rgb blank;
  int ansi_bg;
  int ansi_fg;
  int interval = get_screen_interval(width, height);
  int value_length = 4;

  if (color_type == 2) {
    value_length = 3;
  }

  for (png_uint_32 row = 0; row < height; row += (interval * 2)) {
    for (png_uint_32 col = 0; col < (width * value_length);
        col += (interval * value_length)) {
      bg.red = row_pointers[row][col+0];
      bg.green = row_pointers[row][col+1];
      bg.blue = row_pointers[row][col+2];

      if (row + interval > height) {
        fg = blank;
      } else {
        fg.red = row_pointers[row + interval][col+0];
        fg.green = row_pointers[row + interval][col+1];
        fg.blue = row_pointers[row + interval][col+2];
      }

      if (truecolor) {
        print_truecolor_char(bg, fg);
      } else {
        ansi_bg = get_ansi_color_code(bg);
        ansi_fg = get_ansi_color_code(fg);
        print_ansi_char(ansi_bg, ansi_fg);
      }
    }
    printf("\n");
  }
}

void print_truecolor_char(struct rgb bg, struct rgb fg) {
  printf("\x1B[48;2;%d;%d;%dm", bg.red, bg.green, bg.blue);
  printf("\x1B[38;2;%d;%d;%dm\u2584\x1B[0m", fg.red, fg.green, fg.blue);
}

void print_ansi_char(int ansi_bg, int ansi_fg) {
  printf("\x1B[48;5;%dm", ansi_bg);
  printf("\x1B[38;5;%dm\u2584\x1B[0m", ansi_fg);
}

int get_screen_interval(png_uint_32 width, png_uint_32 height) {
  int interval = 1;
  struct winsize w; 

  /* Get screen row/col */
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  if ( (width > w.ws_col) && (width - w.ws_col) < (height - w.ws_row)) {
    interval = round(width / (double)w.ws_col);
  } else if (height > w.ws_row) {
    interval = round(height / (double)w.ws_row);
  }

  return interval;
}

int get_ansi_color_code(struct rgb values) {
  int ansi_code = 16;

  /*
   * 42.57 comes from 255 / 5.99
   * If you look at the 8-bit ansi color codes, we have 216 colors
   * blue increases every 1 value
   * green increases every 6 values
   * red increases every 36
   * each with a maximum of 6 values, 6 * 6 * 6 = 216
   * https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
   */
  int red = (int) floor(values.red / 42.57);
  int green = (int) floor(values.green / 42.57);
  int blue = (int) floor(values.blue / 42.57);

  ansi_code += blue;
  ansi_code += (red * 36);
  ansi_code += (green * 6);

  return ansi_code;
}

int prechecks(char *file_name, FILE **fp) {
  int is_png;

  /* Can we open the file */
  if (!*fp) {
    printf("Error opening file: %s\n", file_name);
    return 0;
  }

  /* Is the file a PNG? */
  is_png = check_if_png(&*fp);
  if (!is_png) {
    printf("Error file: %s, is not a PNG file\n", file_name);
    return 0;
  }

  return 1;
}

/* Most of this function is from libpng-1.5.13/example.c */
int check_if_png(FILE **fp){
  unsigned char header[PNG_BYTES_TO_CHECK];

  if (!*fp)
    return 0;

  /* Read in some of the signature bytes */
  if(fread(header, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK)
    return 0;

  /* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
  Return nonzero(true) if they match */

  return(!png_sig_cmp(header, (png_size_t)0, PNG_BYTES_TO_CHECK));
}

