/**
  *  Worg
  */
#include <math.h>
#include <stdlib.h>

#define HEADER_SIZE 54
//---------------------- Task (7) -----------------------//
//  Let's build a structure to define a "window".
//  Each window is uniquely determined by the coordinate of the top-left corner, the height and the width.
typedef struct {
  unsigned int x, y;
  unsigned int height, width;
  double correlation;
  int color_index;
} Window;

typedef struct {
  int size, allocated_size;
  Window* v;
} WindowArray;

//  Function that appends a window to an array of windows.
//  The concept is similar to the stl::vector in C++.
void Append(WindowArray *window_array, Window *window) {
  window_array->size++;
  if (window_array->size > window_array->allocated_size) {
    window_array->allocated_size *= 2;
    window_array->v = realloc(window_array->v, sizeof(Window) * window_array->allocated_size);
  }
  window_array->v[window_array->size - 1] = *window;
}

//  We need a function that returns the greyscale version of an image.
BMPImage GetGreyscale(const BMPImage *image) {
  BMPImage greyscale_image;

  for (int i = 0; i < HEADER_SIZE; i++) {
    greyscale_image.header[i] = image->header[i];
  }
  greyscale_image.width = image->width;
  greyscale_image.height = image->height;
  greyscale_image.pixels = (Pixel*) malloc(sizeof(Pixel) * image->width * image->height);

  //  Use the given formula to turn the pixels "gray".
  for (int i = 0; i < image->height * image->width; i++) {
    double value = (double)0.299 * image->pixels[i].rgb_code[2] + (double)0.587 * image->pixels[i].rgb_code[1] + (double)0.114 * image->pixels[i].rgb_code[0];
    greyscale_image.pixels[i].rgb_code[0] = greyscale_image.pixels[i].rgb_code[1] = greyscale_image.pixels[i].rgb_code[2] = (unsigned char) value;
  }

  return greyscale_image;
}

//  Build the function that computes the cross correlation coefficient between the window of an image and a pattern.
//  Note : the window and the pattern have the same dimensions.
double GetCrossCorrelation(BMPImage* image, BMPImage* pattern) {
  int n = image->height * image->width;

  //  Compute the medium intensity for the pattern and for the image.

  double pattern_medium_intensity = 0.0;  //  or s_medium as it is defined in the description of the project
  for (int i = 0; i < pattern->height; i++) {
    for (int j = 0; j < pattern->width; j++) {
      pattern_medium_intensity += pattern->pixels[GetIndex(i, j, pattern->width)].rgb_code[0];
    }
  }
  pattern_medium_intensity /= ((double)pattern->height * pattern->width);

  double image_medium_intensity = 0.0;  //  or f_medium as it is defined in the description of the projects
  for (int i = 0; i < image->height; i++) {
    for (int j = 0; j < image->width; j++) {
      image_medium_intensity += image->pixels[GetIndex(i, j, image->width)].rgb_code[0];
    }
  }
  image_medium_intensity /= ((double)image->height * image->width);

  //  Compute the standard deviation for the pattern and the image.

  double pattern_standard_deviation = 0.0;
  double x = 0.0;
  for (int i = 0; i < pattern->height; i++) {
    for (int j = 0; j < pattern->width; j++) {
      double y = pattern->pixels[GetIndex(i, j, pattern->width)].rgb_code[0] - pattern_medium_intensity;
      x += y * y;
    }
  }
  x /= (double)(n - 1);
  pattern_standard_deviation = sqrt(x);

  double image_standard_deviation= 0.0;
  x = 0.0;
  for (int i = 0; i < image->height; i++) {
    for (int j = 0; j < image->width; j++) {
      double y = image->pixels[GetIndex(i, j, image->width)].rgb_code[0] - image_medium_intensity;
      x += y * y;
    }
  }
  x /= (double)(n - 1);
  image_standard_deviation = sqrt(x);

  double cross_correlation = 0.0;
  for (int i = 0; i < image->height; i++) {
    for (int j = 0; j < image->width; j++) {
      double a = pattern->pixels[GetIndex(i, j, pattern->width)].rgb_code[0];
      double b = image->pixels[GetIndex(i, j, image->width)].rgb_code[0];

      cross_correlation += (a - pattern_medium_intensity) * (b - image_medium_intensity);
    }
  }

  cross_correlation /= (double)n;
  cross_correlation /= image_standard_deviation;
  cross_correlation /= pattern_standard_deviation;


  return cross_correlation;
}

//  The template-matching function, which takes an image, a template and a coefficient and returns the Windows
//  where the matching-coefficient is bigger or equal than the give coefficient.
WindowArray TemplateMatching(BMPImage *image, BMPImage *pattern, const double coefficient) {
  //  Get the greyscale
  BMPImage greyscale_image = GetGreyscale(image);
  BMPImage greyscale_pattern = GetGreyscale(pattern);

  //  Try to position the pattern in every possible position.
  //  The upper left corner can be anywhere in the rectangle [(0, 0), (image.height - pattern.height, image.width - pattern.width)]
  BMPImage window_image;

  window_image.width = pattern->width;
  window_image.height = pattern->height;
  window_image.pixels = (Pixel*) malloc(sizeof(Pixel) * window_image.height * window_image.width);

  WindowArray valid_windows;
  valid_windows.size = 0; valid_windows.allocated_size = 1;
  valid_windows.v = (Window*) malloc(sizeof(Window));

  for (int i = 0; i <= image->height - pattern->height; i++) {
    for (int j = 0; j <= image->width - pattern->width; j++) {
      //  Get the window_image pixels
      for (int row = i; row < i + pattern->height; row++) {
        for (int column = j; column < j + pattern->width; column++) {
          window_image.pixels[GetIndex(row - i, column - j, pattern->width)] = image->pixels[GetIndex(row, column, image->width)];
        }
      }

      double correlation = GetCrossCorrelation(&window_image, pattern);

      if (correlation >= coefficient) {  //  If the current window is valid then we can add it to the solution.
        Window window;
        window.x = i;
        window.y = j;
        window.height = pattern->height;
        window.width = pattern->width;
        window.correlation = correlation;

        Append(&valid_windows, &window);
      }
    }
  }

  free(window_image.pixels);
  free(greyscale_image.pixels);
  free(greyscale_pattern.pixels);

  return valid_windows;
}

//---------------------- Task (8) -----------------------//

void DrawRectangle(BMPImage *image, Window window, Pixel color) {
  //  Draw the upper and lower sides
  for (int j = window.y; j < window.y + window.width; j++) {
    image->pixels[GetIndex(window.x, j, image->width)] = color;
    image->pixels[GetIndex(window.x + window.height - 1, j, image->width)] = color;
  }

  //  Draw the left and right sides
  for (int i = window.x; i < window.x + window.height; i++) {
    image->pixels[GetIndex(i, window.y, image->width)] = color;
    image->pixels[GetIndex(i, window.y + window.width - 1, image->width)] = color;
  }
}

//---------------------- Task (9) -----------------------//
int CompareWindows(const void *a, const void *b) {
  Window w1 = *(Window*)a;
  Window w2 = *(Window*)b;

  return w1.correlation < w2.correlation;
}

void SortWindows(WindowArray *window_array) {
  qsort(window_array->v, window_array->size, sizeof(Window), CompareWindows);
}
//---------------------- Task (10) -----------------------//

int Min(int a, int b) {
  if (a <= b) {
    return a;
  }
  return b;
}

int Max(int a, int b) {
  if (a >= b) {
    return a;
  }
  return b;
}

//  Given two cell segments [x0, x1] and [x2, x3], compute the length of their intersection.
int ComputeOverlap(int x0, int x1, int x2, int x3) {
  int a = Min(x1, x3), b = Max(x0, x2);

  if (a < b) {
    return 0;
  } else {
    return a - b + 1;
  }
}

double ComputeOverlapCoefficient(const Window *w1, const Window *w2) {
  int x0 = w1->x, x1 = w1->x + w1->height - 1;
  int x2 = w2->x, x3 = w2->x + w2->height - 1;

  int overlap_x = ComputeOverlap(x0, x1, x2, x3);

  int y0 = w1->y, y1 = w1->y + w1->width - 1;
  int y2 = w2->y, y3 = w2->y + w2->width - 1;

  int overlap_y = ComputeOverlap(y0, y1, y2, y3);

  double overlap_area = overlap_x * overlap_y;

  double rectangle_area = w1->height * w1->width;
  double coefficient = overlap_area / (2 * rectangle_area - overlap_area);

  return coefficient;
}

//  Function that deletes a given element from an array of windows.
void DeleteElement(WindowArray *window_array, int index) {
  for (int i = index; i + 1 < window_array->size; i++) {
    window_array->v[i] = window_array->v[i + 1];
  }
  window_array->size--;
}

//  Function that detects the overlapping windows and deletes the redundant ones, using a given coefficient.
//  coefficient = 0.2
void DeleteRedundantDetections(WindowArray *window_array) {
  for (int i = 0; i < window_array->size; i++) {
    for (int j = window_array->size - 1; j > i; j--) {
      double coefficient = ComputeOverlapCoefficient(&window_array->v[i], &window_array->v[j]);

      if (coefficient > 0.2) {
        DeleteElement(window_array, j);
      }
    }
  }
}