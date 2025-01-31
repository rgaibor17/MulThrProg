#include <stdlib.h>
#include "bmp.h"
#include "filter.h"

int main(int argc, char **argv) {
  FILE* source;
  FILE* dest;
  BMP_Image* imageIn = NULL;
  BMP_Image* imageOut = NULL;
  int numThreads = 4;
  int boxFilter[3][3] = {
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1}
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
  imageOut = createBMPImage(source);
  apply(imageIn, imageOut);

  printBMPHeader(&(imageOut->header));
  printBMPImage(imageOut);
  writeImage("destination.bmp", imageOut);

  freeImage(imageIn);
  freeImage(imageOut);
  fclose(source);
  fclose(dest);

  exit(EXIT_SUCCESS);
}
