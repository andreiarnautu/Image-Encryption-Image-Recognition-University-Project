/**
  *  Worg
  */
#include <stdio.h>
#include <stdlib.h>

#define HEADER_SIZE 54
//---------------------- Task (2) -----------------------//

//  Because each pixel is composed of 3 independent values (red, green & blue), it's easier to work with them stored
//  in a structure.
typedef struct {
  unsigned char rgb_code[3];  //  blue, green, red actually, because of little endian
} Pixel;

//  We'll also use another structure for the BMP Images that we want to store internally. We have to memorise the
//  header, the pixels in a linearized version and the height and width. It's easier to store the height and the width
//  separately rather than having to extract them from the header every time we need them. Even though it might seem
//  that it's not very efficient, note that they occupy very little memory when compared to the pixel array.
typedef struct {
  unsigned char header[HEADER_SIZE];
  int height, width;

  Pixel *pixels;
} BMPImage;

//  Since we have the pixels stored in an array and in this task we need to operate on them as if they were stored
//  in a matrix, we can do so by implementing a function that returns the index in the array of a cell in the matrix.
int GetIndex(int row, int column, int width) {
  return width * row + column;
}

//  We need a function that computes the padding value for an image. Only the width needs to be passed as a parameter.
int GetPadding(int width) {
  int padding = (3 * width) % 4;
  if (padding > 0) {
    padding = 4 - padding;
  }

  return padding;
}

BMPImage LoadBMPImage(char *file_name) {
  FILE *f = fopen(file_name, "rb");

  if (f == NULL) {
    printf("Unul din fisiere este inexistent sau nu a putut fi deschis.\n"); exit(0);
  }

  BMPImage image;

  //  Read the header.
  for (int i = 0; i < HEADER_SIZE; i++) {
    fread(&image.header[i], 1, 1, f);
  }

  //  Read the width and height.
  fseek(f, 18, SEEK_SET);
  fread(&image.width, sizeof(image.width), 1, f);
  fread(&image.height, sizeof(image.height), 1, f);
  //  Get the padding value
  int padding = GetPadding(image.width);

  //  Read all the pixels and store them in an array.
  image.pixels = (Pixel*) malloc(image.height * image.width * sizeof(Pixel));
  fseek(f, HEADER_SIZE, SEEK_SET);

  for (int i = image.height - 1; i >= 0; i--) {
    for (int j = 0; j < image.width; j++) {
      fread(image.pixels[GetIndex(i, j, image.width)].rgb_code, 3, 1, f);
    }

    fseek(f, padding, SEEK_CUR);
  }

  fclose(f);

  return image;
}

//---------------------- Task (3) -----------------------//

void PrintBMPImage(BMPImage *image, char *file_name) {
  FILE *f = fopen(file_name, "wb");

  if (f == NULL) {
    printf("Unul din fisiere este inexistent sau nu a putut fi deschis.\n"); exit(0);
  }

  //  Print the header.
  for (int i = 0; i < HEADER_SIZE; i++) {
    fwrite(&image->header[i], 1, 1, f);
  }

  //  Print the pixels and also don't forget about the padding.
  int padding = GetPadding(image->width);

  for (int i = image->height - 1; i >= 0; i--) {
    for (int j = 0; j < image->width; j++) {
      fwrite(image->pixels[GetIndex(i, j, image->width)].rgb_code, 3, 1, f);
    }

    unsigned char empty = 0;
    for (int j = 0; j < padding; j++) {
      fwrite(&empty, 1, 1, f);
    }
  }

  fclose(f);
}