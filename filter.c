#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

void apply(BMP_Image * imageIn, BMP_Image * imageOut) {
    int width = imageIn->header.width_px;
    int height = imageIn->norm_height;

    // Box filter size (3x3)
    int boxFilter[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };
    int filterSize = 3;
    int halfFilter = filterSize / 2;

    // Iterate over each pixel in the input image (ignoring the borders for now)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int rSum = 0, gSum = 0, bSum = 0, aSum = 0;
            int count = 0;

            // Apply the 3x3 box filter (neighboring pixels)
            for (int ky = -halfFilter; ky <= halfFilter; ky++) {
                for (int kx = -halfFilter; kx <= halfFilter; kx++) {
                    int nx = x + kx;
                    int ny = y + ky;

                    // Check bounds for pixel location
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        // Access the pixel using the Pixel structure
                        Pixel *currentPixel = &imageIn->pixels[ny][nx];

                        // Accumulate the color values (RGBA)
                        rSum += currentPixel->red * boxFilter[ky + 1][kx + 1];
                        gSum += currentPixel->green * boxFilter[ky + 1][kx + 1];
                        bSum += currentPixel->blue * boxFilter[ky + 1][kx + 1];
                        aSum += currentPixel->alpha * boxFilter[ky + 1][kx + 1];
                        count++;
                    }
                }
            }

            // Set the new pixel value in the output image
            Pixel *outPixel = &imageOut->pixels[y][x];
            outPixel->red = rSum / count;
            outPixel->green = gSum / count;
            outPixel->blue = bSum / count;
            outPixel->alpha = aSum / count;  // Assuming alpha blending is desired
        }
    }
}

typedef struct {
    BMP_Image *imageIn;
    BMP_Image *imageOut;
    int boxFilter[3][3];
    int startRow;
    int endRow;
} ThreadArgs;

void *filterThreadWorker(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    BMP_Image *imageIn = threadArgs->imageIn;
    BMP_Image *imageOut = threadArgs->imageOut;
    int boxFilter[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            boxFilter[i][j] = threadArgs->boxFilter[i][j];
        }
    }
    
    int width = imageIn->header.width_px;
    int height = imageIn->norm_height;

    // Apply the box filter to the assigned portion of the image (rows from startRow to endRow)
    for (int y = threadArgs->startRow; y < threadArgs->endRow; y++) {
        for (int x = 0; x < width; x++) {
            int rSum = 0, gSum = 0, bSum = 0, aSum = 0;
            int count = 0;

            // Apply the 3x3 box filter (neighboring pixels)
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int nx = x + kx;
                    int ny = y + ky;

                    // Check bounds for pixel location
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        // Access the pixel using the Pixel structure
                        Pixel *currentPixel = &imageIn->pixels[ny][nx];

                        // Accumulate the color values (RGBA)
                        rSum += currentPixel->red * boxFilter[ky + 1][kx + 1];
                        gSum += currentPixel->green * boxFilter[ky + 1][kx + 1];
                        bSum += currentPixel->blue * boxFilter[ky + 1][kx + 1];
                        aSum += currentPixel->alpha * boxFilter[ky + 1][kx + 1];
                        count++;
                    }
                }
            }

            // Set the new pixel value in the output image
            Pixel *outPixel = &imageOut->pixels[y][x];
            outPixel->red = rSum / count;
            outPixel->green = gSum / count;
            outPixel->blue = bSum / count;
            outPixel->alpha = aSum / count;  // Assuming alpha blending is desired
        }
    }

    pthread_exit(NULL);
}

void applyParallel(BMP_Image *imageIn, BMP_Image *imageOut, int boxFilter[3][3], int numThreads) {
    int width = imageIn->header.width_px;
    int height = imageIn->norm_height;

    // Divide the image into rows and assign each thread a portion of rows
    pthread_t *threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));
    ThreadArgs *threadArgs = (ThreadArgs *)malloc(numThreads * sizeof(ThreadArgs));

    int rowsPerThread = height / numThreads;

    // Common to all threads
    for (int i = 0; i < numThreads; i++) {
        threadArgs[i].imageIn = imageIn;
        threadArgs[i].imageOut = imageOut;
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                threadArgs[i].boxFilter[j][k] = boxFilter[j][k];
            }
        }

        threadArgs[i].startRow = i * rowsPerThread;
        threadArgs[i].endRow = (i == numThreads - 1) ? height : (i + 1) * rowsPerThread; // Avoid jumping to a non-existing index

        // Create the thread to process the assigned rows
        pthread_create(&threads[i], NULL, filterThreadWorker, (void *)&threadArgs[i]);
    }

    // Wait for all threads to finish processing
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    free(threads);
    free(threadArgs);
}
