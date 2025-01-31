#include <stdlib.h>
#include "bmp.h"
#include "filter.h"

int main(int argc, char **argv) {
  FILE* source;
  FILE* dest1;
  FILE* dest2;
  BMP_Image* imageIn = NULL;
  BMP_Image* imageOut1 = NULL;
  BMP_Image* imageOut2 = NULL;
  int numThreads = 4;
  int boxFilter1[3][3] = {
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1}
  };

  int boxFilter2[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
  };

  if (argc != 4) {
    printError(ARGUMENT_ERROR);
    exit(EXIT_FAILURE);
  }
  
  // Open the source file for reading in binary mode
  if((source = fopen(argv[1], "rb")) == NULL) {
    printError(FILE_ERROR);
    exit(EXIT_FAILURE);
  }
  // Open the destination files for writing in binary mode
  if((dest1 = fopen(argv[2], "wb")) == NULL) {
    printError(FILE_ERROR);
    exit(EXIT_FAILURE);
  }
  if((dest2 = fopen(argv[3], "wb")) == NULL) {
    printError(FILE_ERROR);
    exit(EXIT_FAILURE);
  }

  readImage(source, &imageIn);

  imageOut1 = createBMPImage(source);
  apply(imageIn, imageOut1, boxFilter1);

  imageOut2 = createBMPImage(source);
  applyParallel(imageIn, imageOut2, boxFilter2, numThreads);

  printBMPHeader(&(imageOut1->header));
  printBMPImage(imageOut1);
  writeImage(argv[2], dest1, imageOut1);

  printBMPHeader(&(imageOut2->header));
  printBMPImage(imageOut2);
  writeImage(argv[3], dest2, imageOut2);

  freeImage(imageIn);
  freeImage(imageOut1);
  freeImage(imageOut2);

  fclose(source);
  fclose(dest1);
  fclose(dest2);

  exit(EXIT_SUCCESS);
}
