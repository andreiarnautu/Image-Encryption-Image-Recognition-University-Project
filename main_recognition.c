/**
  *  Worg
  */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common_data.h"
#include "image_recognition.h"

int main() {
  //  Subtask (4)
  char file_name[100];
  char template_name[100];

  //  We'll read the data from this task from a .txt file, because it's easier to introduce the data
  FILE *f = fopen("data.txt", "r");
  //  Read the file path and load the image
  fgets(file_name, 100, f);

  file_name[strlen(file_name) - 1] = '\0';

  BMPImage image = LoadBMPImage(file_name);

  //  Read the number of templates
  int template_number; fscanf(f, "%d ", &template_number);
  //  Read the templates
  BMPImage* image_templates = (BMPImage*) malloc(sizeof(BMPImage) * template_number);

  for (int i = 0; i < template_number; i++) {
    fgets(template_name, 101, f);
    template_name[strlen(template_name) - 1] = '\0';
    image_templates[i] = LoadBMPImage(template_name);
  }

  //  Read the colors that will be used to draw the rectangles. Each pattern has its own color.
  Pixel* colors = (Pixel*) malloc(sizeof(Pixel) * template_number);
  for (int i = 0; i < template_number; i++) {
    unsigned int blue, green, red;
    fscanf(f, "%u%u%u", &blue, &green, &red);
    colors[i].rgb_code[0] = blue;
    colors[i].rgb_code[1] = green;
    colors[i].rgb_code[2] = red;
  }

  //  Start template matching.
  //  All the windows that match will be stored in a WindowArray structure.
  WindowArray valid_windows;
  valid_windows.size = 0;
  valid_windows.allocated_size = 1;
  valid_windows.v = (Window*) malloc(sizeof(Window));

  for (int i = 0; i < 10; i++) {
    printf("Matching pattern number %d...\n", i);
    WindowArray valid_windows_here = TemplateMatching(&image, &image_templates[i], 0.5);
    for (int j = 0; j < valid_windows_here.size; j++) {
      valid_windows_here.v[j].color_index = i;
      Append(&valid_windows, &valid_windows_here.v[j]);
    }
    //  Deallocate the memory.
    free(valid_windows_here.v);
  }


  //  Subtask (5)

  //  Sort the windows and delete the redundant ones.
  SortWindows(&valid_windows);
  DeleteRedundantDetections(&valid_windows);

  //  Draw the rectangles.
  for (int i = 0; i < valid_windows.size; i++) {
    DrawRectangle(&image, valid_windows.v[i], colors[valid_windows.v[i].color_index]);
  }
  //  Print the result back in the "detection.bmp" file.
  PrintBMPImage(&image, "detection.bmp");

  free(image.pixels);
  for (int i = 0; i < template_number; i++) {
    free(image_templates[i].pixels);
  }
  free(image_templates);
  free(colors);
  free(valid_windows.v);

  fclose(f);
}