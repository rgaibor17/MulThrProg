#include <stdlib.h>
#include "bmp.h"
#include "filter.h"

int main(int argc, char **argv) {
  FILE* source;
  FILE* dest;
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

  if (argc != 3) {
    printError(ARGUMENT_ERROR);
    exit(EXIT_FAILURE);
  }
  
  if((source = fopen(argv[1], "rb")) == NULL) {
    printError(FILE_ERROR);
    exit(EXIT_FAILURE);
  }
  if((dest = fopen(argv[2], "wb")) == NULL) {
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
  writeImage("destination1.bmp", imageOut1);

  printBMPHeader(&(imageOut2->header));
  printBMPImage(imageOut2);
  writeImage("destination2.bmp", imageOut2);

  freeImage(imageIn);
  freeImage(imageOut1);
  freeImage(imageOut2);

  fclose(source);
  fclose(dest);

  exit(EXIT_SUCCESS);
}
