#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function to apply a 3x3 blur filter on a single pixel. */
void applyFilter(BMP_Image *imageIn, BMP_Image *imageOut, int x, int y) {
    int sumRed = 0, sumGreen = 0, sumBlue = 0;
    int count = 0;

    // 3x3 kernel loop (including the pixel itself and its 8 neighbors)
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nx = x + i, ny = y + j;
            if (nx >= 0 && nx < imageIn->header.width_px && ny >= 0 && ny < imageIn->norm_height) {
                sumRed += imageIn->pixels[ny][nx].red;
                sumGreen += imageIn->pixels[ny][nx].green;
                sumBlue += imageIn->pixels[ny][nx].blue;
                count++;
            }
        }
    }

    // Set the blurred pixel to the average color value
    imageOut->pixels[y][x].red = sumRed / count;
    imageOut->pixels[y][x].green = sumGreen / count;
    imageOut->pixels[y][x].blue = sumBlue / count;
}

/* Function to apply the blur filter to the entire image. */
void apply(BMP_Image *imageIn, BMP_Image *imageOut) {
    // Loop through all pixels and apply the filter
    for (int y = 0; y < imageIn->norm_height; y++) {
        for (int x = 0; x < imageIn->header.width_px; x++) {
            applyFilter(imageIn, imageOut, x, y);
        }
    }
}

/* Struct to pass arguments to each thread. */
typedef struct {
    BMP_Image *imageIn;
    BMP_Image *imageOut;
    int startRow;
    int endRow;
} ThreadArgs;

/* Worker function for each thread. This will process a slice of the image. */
void *filterThreadWorker(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    BMP_Image *imageIn = threadArgs->imageIn;
    BMP_Image *imageOut = threadArgs->imageOut;

    for (int y = threadArgs->startRow; y < threadArgs->endRow; y++) {
        for (int x = 0; x < imageIn->header.width_px; x++) {
            applyFilter(imageIn, imageOut, x, y);
        }
    }

    return NULL;
}

/* Function to apply the blur filter to the entire image in parallel. */
void applyParallel(BMP_Image *imageIn, BMP_Image *imageOut, int boxFilter[3][3], int numThreads) {
    pthread_t *threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));
    ThreadArgs *threadArgs = (ThreadArgs *)malloc(numThreads * sizeof(ThreadArgs));

    int rowsPerThread = imageIn->norm_height / numThreads;
    for (int i = 0; i < numThreads; i++) {
        threadArgs[i].imageIn = imageIn;
        threadArgs[i].imageOut = imageOut;
        threadArgs[i].startRow = i * rowsPerThread;
        threadArgs[i].endRow = (i == numThreads - 1) ? imageIn->norm_height : (i + 1) * rowsPerThread; // Avoid jumping to a non-existing index

        // Create threads
        pthread_create(&threads[i], NULL, filterThreadWorker, (void *)&threadArgs[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    free(threads);
    free(threadArgs);
}

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

    readImage(source, imageIn);
    if(!checkBMPValid(&imageIn->header)) {
        printError(VALID_ERROR);
        exit(EXIT_FAILURE);
    }

    imageOut = createBMPImage(dest);
    applyParallel(imageIn, imageOut, boxFilter, numThreads);

    readImage(source, imageOut);
    printBMPHeader(&imageOut->header);
    printBMPImage(imageOut);
    writeImage("destination.bmp", imageOut);

    freeImage(imageIn);
    freeImage(imageOut);
    fclose(source);
    fclose(dest);

    exit(EXIT_SUCCESS);
}
