/**
  *  Worg
  */
#include <stdio.h>
#include <stdlib.h>

//#include "common_data.h"

#define HEADER_SIZE 54

//---------------------- Task (1) -----------------------//

//  The function generates the next random number of a series of random numbers.
//  It is solely based on the last number generated, or on the seed if it is the first number generated.
unsigned int XorShift32(unsigned int former_state) {
  unsigned int current_state = former_state;
  current_state = current_state ^ current_state << 13;
  current_state = current_state ^ current_state >> 17;
  current_state = current_state ^ current_state << 5;
  return current_state;
}

//---------------------- Task (4) -----------------------//
//  A function that generates an random-number array of a given size.
unsigned int* GetRandomNumbers(unsigned int seed, int counter) {
  unsigned int* random_numbers = (unsigned int*) malloc(counter * sizeof(int));

  random_numbers[0] = seed;
  for (int i = 1; i < counter; i++) {
    random_numbers[i] = XorShift32(random_numbers[i - 1]);
  }

  return random_numbers;
}

unsigned int* GeneratePermutation(int size, unsigned int *random_numbers) {
  unsigned int* permutation = (unsigned int*) malloc(size * sizeof(unsigned int));

  for (unsigned int i = 0; i < size; i++) {
    permutation[i] = i;
  }

  int cursor = 1;
  for (int i = size - 1; i > 0; i--) {
    unsigned int index = random_numbers[cursor] % (i + 1);
    //  Swap permutation[i] with permutation[index].
    unsigned int aux = permutation[i];
    permutation[i] = permutation[index];
    permutation[index] = aux;

    cursor++;
  }

  return permutation;
}

Pixel XorPixelWithConstant(Pixel pixel, unsigned int x) {
  Pixel result = pixel;

  unsigned int x0 = x & 255;
  x = x >> 8;

  unsigned int x1 = x & 255;
  x = x >> 8;

  unsigned int x2 = x & 255;

  //  Blue
  result.rgb_code[0] ^= (unsigned char)x0;
  //  Green
  result.rgb_code[1] ^= (unsigned char)x1;
  //  Red
  result.rgb_code[2] ^= (unsigned char)x2;

  return result;
}

Pixel XorPixelWithPixel(Pixel p1, Pixel p2) {
  Pixel result;
  for (int i = 0; i < 3; i++) {
    result.rgb_code[i] = p1.rgb_code[i] ^ p2.rgb_code[i];
  }

  return result;
}

void EncryptImage(char* image_file, char* encrypted_image_file, char* key_file) {
  //  Open all the files
  FILE *f = fopen(key_file, "r");

  if (f == NULL) {
    printf("Unul din fisiere este inexistent sau nu a putut fi deschis.\n"); exit(0);
  }

  unsigned int secret_key; fscanf(f, "%u", &secret_key);

  BMPImage image = LoadBMPImage(image_file);

  unsigned int* random_numbers = GetRandomNumbers(secret_key, 2 * image.width * image.height);
  unsigned int* random_permutation = GeneratePermutation(image.height * image.width, random_numbers);

  BMPImage encrypted_image;

  //  The encrypted image has the same header and dimensions, so let's copy them.
  for (int i = 0; i < HEADER_SIZE; i++) {
    encrypted_image.header[i] = image.header[i];
  }
  encrypted_image.height = image.height;
  encrypted_image.width = image.width;
  encrypted_image.pixels = (Pixel*) malloc(sizeof(Pixel) * encrypted_image.width * encrypted_image.height);

  //  Now let's rotate the pixels using the generated permutation.
  for (int i = 0; i < encrypted_image.height * encrypted_image.width; i++) {
    encrypted_image.pixels[random_permutation[i]] = image.pixels[i];
  }

  //  Read the starting value from the .txt file
  unsigned int starting_value;
  fscanf(f, "%u", &starting_value);

  //  cursor = the index of the random number we are currently using for encryption
  int cursor = image.width * image.height;

  //  The first pixel has a separate encryption formula, so we'll compute it separately.
  encrypted_image.pixels[0] = XorPixelWithConstant(encrypted_image.pixels[0], starting_value);
  encrypted_image.pixels[0] = XorPixelWithConstant(encrypted_image.pixels[0], random_numbers[cursor]);
  cursor++;

  //  Now let's encrypt the remaining w * h - 1 pixels
  for (int i = 1; i < encrypted_image.height * encrypted_image.width; i++) {
    encrypted_image.pixels[i] = XorPixelWithPixel(encrypted_image.pixels[i], encrypted_image.pixels[i - 1]);
    encrypted_image.pixels[i] = XorPixelWithConstant(encrypted_image.pixels[i], random_numbers[cursor]);
    cursor++;
  }

  //  Print the encrypted image
  PrintBMPImage(&encrypted_image, encrypted_image_file);

  //  Deallocate the memory.
  free(random_numbers);
  free(random_permutation);
  free(image.pixels);
  free(encrypted_image.pixels);

  //  Don't forget to close the file
  fclose(f);
}

//---------------------- Task (5) -----------------------//
void DecryptImage(char *encrypted_image_file, char *decrypted_image_file, char *key_file) {
  //  Open all the files
  FILE *f = fopen(key_file, "r");

  if (f == NULL) {
    printf("Unul din fisiere este inexistent sau nu a putut fi deschis.\n"); exit(0);
  }

  unsigned int secret_key; fscanf(f, "%u", &secret_key);
  unsigned int starting_value; fscanf(f, "%u", &starting_value);

  BMPImage encrypted_image = LoadBMPImage(encrypted_image_file);

  unsigned int* random_numbers = GetRandomNumbers(secret_key, 2 * encrypted_image.width * encrypted_image.height);
  unsigned int* random_permutation = GeneratePermutation(encrypted_image.height * encrypted_image.width, random_numbers);

  BMPImage decrypted_image;

  //  The decrypted image has the same header and dimensions, so let's copy them.
  for (int i = 0; i < HEADER_SIZE; i++) {
    decrypted_image.header[i] = encrypted_image.header[i];
  }
  decrypted_image.height = encrypted_image.height;
  decrypted_image.width = encrypted_image.width;
  decrypted_image.pixels = (Pixel*) malloc(sizeof(Pixel) * decrypted_image.width * decrypted_image.height);

  //  Decrypting an image is very similar to encrypting it.
  //  Just reverse the order of operations (do the xor operations first, then change the order using the permutation).

  //  cursor = the index of the random number we are currently using for encryption
  int cursor = 2 * encrypted_image.width * encrypted_image.height - 1;

  //  Now let's encrypt the remaining w * h - 1 pixels
  for (int i = encrypted_image.height * encrypted_image.width - 1; i > 0; i--) {
    encrypted_image.pixels[i] = XorPixelWithConstant(encrypted_image.pixels[i], random_numbers[cursor]);
    encrypted_image.pixels[i] = XorPixelWithPixel(encrypted_image.pixels[i], encrypted_image.pixels[i - 1]);
    cursor--;
  }

  //  The first pixel has a separate encryption formula, so we'll compute it separately.
  encrypted_image.pixels[0] = XorPixelWithConstant(encrypted_image.pixels[0], starting_value);
  encrypted_image.pixels[0] = XorPixelWithConstant(encrypted_image.pixels[0], random_numbers[cursor]);

  //  Rotate the pixels using the generated permutation
  for (int i = 0; i < decrypted_image.height * decrypted_image.width; i++) {
    decrypted_image.pixels[i] = encrypted_image.pixels[random_permutation[i]];
  }

  PrintBMPImage(&decrypted_image, decrypted_image_file);

  //  Deallocate the memory.
  free(random_numbers);
  free(random_permutation);
  free(encrypted_image.pixels);
  free(decrypted_image.pixels);

  //  Close the file
  fclose(f);
}

//---------------------- Task (6) -----------------------//
void PrintBMPTest(char *file_name) {
  BMPImage image = LoadBMPImage(file_name);

  printf("//------------------------ ------------------------//\n");

  printf("Rezultatele testului pe fiecare canal de culoare sunt:\n");

  //  Compute the test for each of the 3 colors
  for (int color = 0; color < 3; color++) {
    double chi_squared = 0;

    double f_average = (double)image.width * (double)image.height / (double)256;

    double* f = (double*) malloc(256 * sizeof(double));
    for (int i = 0; i < 256; i++) {
      f[i] = 0.0;
    }

    for (int i = 0; i < image.height * image.width; i++) {
      f[image.pixels[i].rgb_code[color]] += 1.0;
    }
    for (int i = 0; i < 256; i++) {
      chi_squared += (f[i] - f_average) * (f[i] - f_average) / f_average;
    }

    free(f);

    if (color == 0) {
      printf("Blue: %.2f\n", chi_squared);
    } else if(color == 1) {
      printf("Green: %.2f\n", chi_squared);
    } else {
      printf("Red: %.2f\n", chi_squared);
    }
  }
  printf("//------------------------ ------------------------//\n");

  //  Deallocate the memory.
  free(image.pixels);
}


void Compare(char *file1, char *file2) {
  BMPImage image_1 = LoadBMPImage(file1);
  BMPImage image_2 = LoadBMPImage(file2);

  for (int i = 0; i < HEADER_SIZE; i++) {
    if (image_1.header[i] != image_2.header[i]) {
      printf("Diferenta la header!---Octetul %d\n", i);
    }
  }

  if (image_1.height != image_2.height) {
    printf("Diferenta la height!---height_1 = %d != %d = height_2\n", image_1.height, image_2.height);
  }

  if (image_1.width != image_2.width) {
    printf("Diferenta la width!---width_1 = %d != %d = width_2\n", image_1.width, image_2.width);
  }

  for (int i = 0; i < image_1.height * image_1.width; i++) {
    if (image_1.pixels[i].rgb_code[0] != image_2.pixels[i].rgb_code[0] ||
        image_1.pixels[i].rgb_code[1] != image_2.pixels[i].rgb_code[1] ||
        image_1.pixels[i].rgb_code[2] != image_2.pixels[i].rgb_code[2]) {

        printf("Diferenta la pixelul %d!\n", i);
    }
  }

  //  Deallocate the memory.
  free(image_1.pixels);
  free(image_2.pixels);
}

void TestXor32() {
  unsigned int seed = 1000;

  for (int i = 0; i < 8; i++) {
    seed = XorShift32(seed);
    printf("%u\n", seed);
  }
}