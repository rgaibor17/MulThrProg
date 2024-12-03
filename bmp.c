#include <stdlib.h>
#include <stdio.h>

#include "bmp.h"
/* USE THIS FUNCTION TO PRINT ERROR MESSAGES
   DO NOT MODIFY THIS FUNCTION
*/
void printError(int error){
  switch(error){
  case ARGUMENT_ERROR:
    printf("Usage:ex5 <source> <destination>\n");
    break;
  case FILE_ERROR:
    printf("Unable to open file!\n");
    break;
  case MEMORY_ERROR:
    printf("Unable to allocate memory!\n");
    break;
  case VALID_ERROR:
    printf("BMP file not valid!\n");
    break;
  default:
    break;
  }
}

/* The input argument is the source file pointer. The function will first construct a BMP_Image image by allocating memory to it.
 * Then the function read the header from source image to the image's header.
 * Compute data size, width, height, and bytes_per_pixel of the image and stores them as image's attributes.
 * Finally, allocate menory for image's data according to the image size.
 * Return image;
*/
BMP_Image* createBMPImage(FILE* fptr) {

  if (fptr == NULL) {
    printError(FILE_ERROR);
    return NULL;
  }

  //Allocate memory for BMP_Image*;
  BMP_Image* bmp = (BMP_Image*)malloc(sizeof(BMP_Image));
  if (bmp == NULL) {
    printError(MEMORY_ERROR);
    free(bmp);
    return NULL;
  }

  //Read the first 54 bytes of the source into the header
  // Read BMP header from file
  if (fread(&(bmp->header), sizeof(BMP_Header), 1, fptr) != 1) {
    free(bmp);
    printError(FILE_ERROR);
    return NULL;
  }

  // Check if the BMP file is valid
  if (!checkBMPValid(&(bmp->header))) {
    free(bmp);
    printError(VALID_ERROR);
    return NULL;
  }

  //Compute data size, width, height, and bytes per pixel
  //Normalizaed height and bytes per pixel
  bmp->bytes_per_pixel = bmp->header.bits_per_pixel / 8;
  bmp->norm_height = abs(bmp->header.height_px);
  int width = bmp->header.width_px;
  int height = bmp->norm_height;
  bmp->header.imagesize = width * height * bmp->bytes_per_pixel;

  // Allocate memory for pixel data
  bmp->pixels = (Pixel**)malloc(height * sizeof(Pixel*)); // Rows
  if (bmp->pixels == NULL) {
    printError(MEMORY_ERROR);
    free(bmp);
    return NULL;
  }

  // Allocate memory for each row of pixels
  for (int i = 0; i < bmp->norm_height; i++) {
    bmp->pixels[i] = (Pixel*)malloc(sizeof(Pixel*));
    if (bmp->pixels[i] == NULL) {
      printError(MEMORY_ERROR);
      
      // Free previously allocated memory before returning
      for (int j = 0; j < i; j++) {
        free(bmp->pixels[j]);
      }
      free(bmp->pixels);
      free(bmp);
      return NULL;
    }
  }
  return bmp;
}

/* The input arguments are the source file pointer, the image data pointer, and the size of image data.
 * The functions reads data from the source into the image data matriz of pixels.
*/
void readImageData(FILE* srcFile, BMP_Image * image, int dataSize) {
  if (srcFile == NULL || image == NULL || dataSize <= 0) {
    fprintf(stderr, "Invalid input arguments.\n");
    return;
  }

  // Iterate through each row of pixels
  for (int i = 0; i < image->norm_height; i++) {
    // Read each pixel in the row
    if (fread(image->pixels[i], sizeof(Pixel), 1, srcFile) != 1) {
      fprintf(stderr, "Error reading pixel at (%d).\n", i);
      return;
    }
  }
}

/* The input arguments are the pointer of the binary file, and the image data pointer.
 * The functions open the source file and call to CreateBMPImage to load de data image.
*/
void readImage(FILE *srcFile, BMP_Image * dataImage) {
  if (srcFile == NULL) {
    fprintf(stderr, "Invalid input arguments: srcFile is NULL.\n");
    return;
  }

  // Reset the file pointer to the beginning of the file
  rewind(srcFile);

  // Call CreateBMPImage to load the image data
  dataImage = createBMPImage(srcFile);
  if (dataImage == NULL) {
    fprintf(stderr, "Failed to create BMP image from source file.\n");
    return;
  }

  // Read the pixel data
  readImageData(srcFile, dataImage, dataImage->header.size);
  // Reset the file pointer to the beginning of the file
  rewind(srcFile);
}

/* The input arguments are the destination file name, and BMP_Image pointer.
 * The function write the header and image data into the destination file.
*/
void writeImage(char* destFileName, BMP_Image* dataImage) {
 if (destFileName == NULL || dataImage == NULL) {
    fprintf(stderr, "Invalid arguments: destFileName or dataImage is NULL.\n");
    return;
  }

  // Open the destination file for writing in binary mode
  FILE* destFile = fopen(destFileName, "wb");
  if (destFile == NULL) {
    printError(FILE_ERROR);
    return;
  }

  // Write the BMP file header
  if (fwrite(&(dataImage->header), sizeof(BMP_Header), 1, destFile) != 1) {
    fprintf(stderr, "Error: Failed to write header.\n");
    fclose(destFile);
    return;
  }

  // Write the pixel data
  int row_size = dataImage->header.width_px * dataImage->bytes_per_pixel;
  printf("row size: %d\n", row_size);

  // Write pixel data row by row
  for (int row = 0; row < dataImage->norm_height; row++) {
    if (fwrite(dataImage->pixels[row], row_size, 1, destFile) != dataImage->header.width_px) {
      fprintf(stderr, "Error: Failed to write pixel data for row %d.\n", row);
      fclose(destFile);
      return;
    }
  }

  // Close the file
  fclose(destFile);
  printf("Image successfully written to %s\n", destFileName);
}

/* The input argument is the BMP_Image pointer. The function frees memory of the BMP_Image.
*/
void freeImage(BMP_Image* image) {
  if (image != NULL) {
    if (image->pixels != NULL) {
      // Free each row of pixels
      for (int i = 0; i < image->norm_height; i++) {
        free(image->pixels[i]);  // Free each row of pixels
      }
      free(image->pixels);
      }
    free(image);
  }
}

/* The functions checks if the source image has a valid format.
 * It returns TRUE if the image is valid, and returns FASLE if the image is not valid.
 * DO NOT MODIFY THIS FUNCTION
*/
int checkBMPValid(BMP_Header* header) {
  // Print the header fields to debug the issue
  printf("Checking BMP header...\n");
  printBMPHeader(header);

  // Check if the BMP file is valid
  if (header->type != 0x4d42) {
    printf("Invalid BMP magic number\n");
    return FALSE;
  }
  if (header->bits_per_pixel != 32) {
    printf("Invalid bits_per_pixel: expected 32, got %d\n", header->bits_per_pixel);
    return FALSE;
  }
  if (header->planes != 1) {
    printf("Invalid planes: expected 1, got %d\n", header->planes);
    return FALSE;
  }
  if (header->compression != 0) {
    printf("Invalid compression: expected 0, got %d\n", header->compression);
    return FALSE;
  }
  return TRUE;
}

/* The function prints all information of the BMP_Header.
   DO NOT MODIFY THIS FUNCTION
*/
void printBMPHeader(BMP_Header* header) {
  printf("file type (should be 0x4d42): %x\n", header->type);
  printf("file size: %d\n", header->size);
  printf("offset to image data: %d\n", header->offset);
  printf("header size: %d\n", header->header_size);
  printf("width_px: %d\n", header->width_px);
  printf("height_px: %d\n", header->height_px);
  printf("planes: %d\n", header->planes);
  printf("bits: %d\n", header->bits_per_pixel);
}

/* The function prints information of the BMP_Image.
   DO NOT MODIFY THIS FUNCTION
*/
void printBMPImage(BMP_Image* image) {
  printf("data size is %ld\n", sizeof(image->pixels));
  printf("norm_height size is %d\n", image->norm_height);
  printf("bytes per pixel is %d\n", image->bytes_per_pixel);
}
