int prechecks(char *file_name, FILE **fp);
int check_if_png(FILE **fp);
void print_image(png_bytep *row_pointers, png_uint_32 width,
    png_uint_32 height, int color_type, int truecolor);
void print_truecolor_char(int red, int green, int blue,
    int background, char *output_str);
void print_ansi_char(int ansi_code, int background, char *output_str);
void set_output_str(char *output_str, int alpha);
int get_ansi_color_code(int red, int green, int blue);
int get_screen_interval(png_uint_32 width, png_uint_32 height);
int get_image_and_info(png_structp png_ptr, png_infop info_ptr,
    png_bytep **row_pointers, png_uint_32 *width, png_uint_32 *height, int *color_type);
