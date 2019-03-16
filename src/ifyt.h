int prechecks(char *file_name, FILE **fp);
int check_if_png(FILE **fp);
int get_screen_interval(png_uint_32 width, png_uint_32 height);
int get_image_and_info(png_structp png_ptr, png_infop info_ptr,
    png_bytep **row_pointers, png_uint_32 *width, png_uint_32 *height, int *color_type);

void print_image(png_bytep *row_pointers, png_uint_32 width,
    png_uint_32 height, int color_type, int truecolor);

struct rgb {
  int red;
  int green;
  int blue;
  int alpha;
};

int is_blank(struct rgb values);
int get_ansi_color_code(struct rgb values);
void print_truecolor_char(struct rgb row1, struct rgb row2);
void print_ansi_char(int ansi_bg, int ansi_fg);
