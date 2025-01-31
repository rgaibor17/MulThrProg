#include <stdlib.h>

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

  //Allocate memory for BMP_Image*
  BMP_Image* bmp = (BMP_Image*)malloc(sizeof(BMP_Image));
  if (bmp == NULL) {
    printError(MEMORY_ERROR);
    free(bmp);
    return NULL;
  }

  // Reset the file pointer to the beginning of the file
  rewind(fptr);

  //Read the first 54 bytes of the source into the header
  fread(&(bmp->header), sizeof(BMP_Header), 1, fptr);

  if (!checkBMPValid(&(bmp->header))) {
    printError(VALID_ERROR);
    exit(EXIT_FAILURE);
  }

  //Compute data size, width, height, and bytes per pixel
  int width = bmp->header.width_px;
  int height = bmp->header.height_px;
  short bitsPerPixel = bmp->header.bits_per_pixel;
  int bytesPerPixel = bitsPerPixel / 8;
  int dataSize = width * abs(height) * bytesPerPixel;

  //Normalizaed height and bytes per pixel
  bmp->norm_height = abs(height);
  bmp->bytes_per_pixel = bytesPerPixel;

  //Allocate memory for image data
  bmp->pixels = (Pixel**)malloc(bmp->norm_height * sizeof(Pixel*));
  if (bmp->pixels == NULL) {
    printError(MEMORY_ERROR);
    free(bmp->pixels);
    free(bmp);
    return NULL;
  }

  // Allocate memory for each row of pixels
  for (int i = 0; i < bmp->norm_height; i++) {
    bmp->pixels[i] = (Pixel*)malloc(width * sizeof(Pixel)); //Rows
    if (!bmp->pixels[i]) {
      printError(MEMORY_ERROR);
      //Free previously allocated memory before returning
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

  // Ensure pixels has been allocated memory
  if (image->pixels == NULL) {
    printError(MEMORY_ERROR);
    return;
  }

  // Iterate through each row of pixels
  for (int row = 0; row < image->norm_height; row++) {
    // Read each pixel in the row
    for (int col = 0; col < image->header.width_px; col++) {
      // Read 4 bytes (one Pixel) into the pixel structure
      if (fread(&(image->pixels[row][col]), sizeof(Pixel), 1, srcFile) != 1) {
        fprintf(stderr, "Error reading pixel at (%d, %d).\n", row, col);
        return;
      }
    }
  }
}

/* The input arguments are the pointer of the binary file, and the image data pointer.
 * The functions open the source file and call to CreateBMPImage to load de data image.
*/
void readImage(FILE *srcFile, BMP_Image ** dataImage) {
  if (srcFile == NULL) {
    fprintf(stderr, "Invalid input arguments: srcFile is NULL.\n");
    return;
  }

  // Call CreateBMPImage to load the image data
  *dataImage = createBMPImage(srcFile);
  if (dataImage == NULL) {
    fprintf(stderr, "Failed to create BMP image from source file.\n");
    return;
  }

  //Call readImageData to fill image object with data from the source file given
  readImageData(srcFile, *dataImage, (*dataImage)->header.size);

  // Reset the file pointer to the beginning of the file
  rewind(srcFile);
}

/* The input arguments are the destination file name, and BMP_Image pointer.
 * The function write the header and image data into the destination file.
*/
void writeImage(char* destFileName, BMP_Image* dataImage) {
 if (destFileName == NULL || dataImage == NULL || dataImage->pixels == NULL) {
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

 // Calculate padding for BMP row alignment (rows must be multiple of 4 bytes)
  int rowPadding = (4 - (dataImage->header.width_px * sizeof(Pixel)) % 4) % 4;
  uint8_t padding[3] = {0, 0, 0}; // Maximum padding is 3 bytes

  // Write pixel data row by row
  for (int row = 0; row < dataImage->norm_height; row++) {
    if (fwrite(dataImage->pixels[row], sizeof(Pixel), dataImage->header.width_px, destFile) != dataImage->header.width_px) {
      fprintf(stderr, "Error: Failed to write pixel data for row %d.\n", row);
      fclose(destFile);
      return;
    }

    // Write padding bytes, if any
    if (rowPadding > 0) {
      if (fwrite(padding, 1, rowPadding, destFile) != rowPadding) {
        fprintf(stderr, "Error: Failed to write padding for row %d.\n", row);
        fclose(destFile);
        return;
      }
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
  // Make sure this is a BMP file
  if (header->type != 0x4d42) {
    return FALSE;
  }
  // Make sure we are getting 24 bits per pixel (Changed to 32 bits as per the assignemnt's instructions)
  if (header->bits_per_pixel != 32) {
    return FALSE;
  }
  // Make sure there is only one image plane
  if (header->planes != 1) {
    return FALSE;
  }
  // Make sure there is no compression
  if (header->compression != 0) {
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
