#include "bmp.h"

void apply(BMP_Image * imageIn, BMP_Image * imageOut, int boxFilter[3][3]);

void applyParallel(BMP_Image * imageIn, BMP_Image * imageOut, int boxFilter[3][3], int numThreads);

void *filterThreadWorker(void * args);