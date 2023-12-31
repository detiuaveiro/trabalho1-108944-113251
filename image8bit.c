/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec:113251  Name: João Ferreira
// NMec:108944  Name: Beatriz Saraiva
// 
// 
// Date: 06/11/2023
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "instrumentation.h"

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
// 
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};


// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
// 
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() { ///
  return errCause;
}


// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success = 
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
// 
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
// 
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
// 
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)


// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}


/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  InstrName[1] = "LocCount"; // InstrCount[0] vai contar LocateSubimages
  
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
#define LocateCompar InstrCount[1]


// TIP: Search for PIXMEM or InstrCount to see where it is incremented!


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

Image ImageCreate(int width, int height, uint8 maxval) {   //---------------- Função escrita dia 6/11/2023
  assert (width >= 0);
  assert (height >= 0);
  assert (0 < maxval && maxval <= PixMax);
  Image img = (Image)malloc(sizeof(struct image));  //Aloca memória para a imagem
  if (img == NULL){                          // Se a alocação de memória para a imagem falhar    
    printf("ERRO: A imagem não existe");       // é impressa uma mensagem de erro
    ImageErrMsg();                             //
    return img;                              // e devolvido o valor NULL
  }                                            

  img->width = width;                        //  
  img->height = height;                      // Atribui width, height e maxval à estrutura da imagem
  img->maxval = maxval;                      //

  img->pixel=(uint8*)malloc(sizeof(uint8)*height*width);    //Aloca memória para os pixeis da imagem
  for (int i = 0; i < width*height; i++){                     //
    img->pixel[i] = 0;                                      //Inicializa todos os pixeis da imagem para a cor preto (0)
  }
                                                              
  return img;                                               
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image* img) {                     //---------------- Função escrita dia 12/11/2023
  assert (img != NULL);
  if (*img != NULL){  
    free((*img)->pixel);        //Liberta a memória alocada para os pixeis
    free(*img);                 //Liberta a memória alocada para a imagem
    *img = NULL;                //Define o ponteiro para NULL
  }
}

/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) { ///
  int w, h;
  int maxval;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  int success = 
  check( (f = fopen(filename, "rb")) != NULL, "Open failed" ) &&
  // Parse PGM header
  check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid file format" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid width" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid height" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval" ) &&
  check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace expected" ) &&
  // Allocate image
  (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
  // Read pixels
  check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading pixels" );
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL) fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) { ///
  assert (img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE* f = NULL;

  int success =
  check( (f = fopen(filename, "wb")) != NULL, "Open failed" ) &&
  check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed" ) &&
  check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing pixels failed" ); 
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f);
  return success;
}


/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert (img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert (img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert (img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) {                      //---------------- Função escrita dia 13/11/2023
  assert (img != NULL);
  *min = img->pixel[0];                                    //define min com o valor do primeiro pixel
  *max = img->pixel[0];                                    //define max com o valor do primeiro pixel
  for (long i = 0; i < img->width*img->height; i++){       // percorre todos os pixeis da imagem
    if (img->pixel[i] < *min){                             //verifica se o valor do pixel é inferior a min
      *min = img->pixel[i];                                //se a condição se verificar atribui a min o valor do pixel 
    }
    else if (img->pixel[i] > *max){                        //se a condição não se verificar compara se o valor do pixel é superior a max
      *max = img->pixel[i];                                //se a condição se verificar atribui a max o valor do pixel
    }
    
    if (*min == 0 && *max == img->maxval){                 //se o min for 0 e max = maxval 
      return;
    }
  }
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert (img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  return (0 <= x && x+w <= img->width) && (0 <= y && y+h <= img->height); //verifica se o retangulo com largura w e altura h e com origem nas coordenadas x e y est+a contido na imagem
}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to 
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel. 
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) {
  int index;
  
  index = x+(y*img->width);               // (44,2) será o pixel[244] se width=100, 44+2*100. Ou seja index = y*largura + x
  assert (0 <= index && index < img->width*img->height);    //garante que o index é valido para a imagem dada.
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (read)
  return img->pixel[G(img, x, y)];
} 

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
} 


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.


/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) {                                             //---------------- Função escrita dia 13/11/2023
  assert (img != NULL);
  for (long i = 0; i < img->width*img->height; i++){    //Percorre todos os pixeis da imagem
    img->pixel[i] = img->maxval - img->pixel[i];     //Subtrai ao maxval o valor do pixel obtendo assim o valor novo para o efeito negativo 
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) {                                  //---------------- Função escrita dia 15/11/2023
  assert (img != NULL);
  for (long i = 0; i < img->width*img->height; i++){           //Percorre todos os pixeis da imagem
    if (img->pixel[i]<thr){                                    //Verifica se o valor do pixel é menor que o valor thr
      img->pixel[i]=0;                                         //Se for transforma o pixel num pixel preto             
    } else {
      img->pixel[i]=img->maxval;                               //Caso contrário iguala o valor do pixel a maxval
    }
  }
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) {                              //---------------- Função escrita dia 18/11/2023
  assert (img != NULL);
  assert (factor >= 0.0);                               //garante que o factor usado não é negativo (o que invertiria as cores da imagem)
  for (long i = 0; i < img->width*img->height; i++){                //Percorre todos os pixeis da imagem
    int NovoValorPixel = img->pixel[i]*factor+0.5;                 //Multplicação do valor de cada pixel pelo factor
    if (NovoValorPixel > img->maxval){                        //Verifica que o novo valor do pixel não excede o valor máximo maxval
      img->pixel[i] = img->maxval;                             //Se exceder então o valor no novo pixel será maxval
    }
    else{
      img->pixel[i] = NovoValorPixel;
    }
  }
}


/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
/// 
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint: 
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees counterclockwise.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) {                                                  //---------------- Função escrita dia 18/11/2023
  assert (img != NULL);
  Image imgRot = ImageCreate(img->height, img->width, img->maxval);      //Cria a imagem rodada (width = img->height e heigh = img->width)
  for (int j = 0; j < img->height; j++){                                //Percorre as colunas
    for (int i = 0; i < img->width; i++){                               //Percorre as linhas
      ImageSetPixel(imgRot,j, ((imgRot->height-1)-i),ImageGetPixel(img,i,j));   //atribui ao pixel (y,altura-x) da imagem rodada o pixel (x,y) da imagem 1 de modo a gerar a imagem rodada no sentido anti-horário
    }
  }
  return imgRot;      //Retorna a imagem rodada
  
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) {                                              //---------------- Função escrita dia 18/11/2023
  assert (img != NULL);
  Image imgMirror = ImageCreate(img->width, img->height, img->maxval);       //Cria a imagem espelhada
  for (int j = 0; j < img->height; j++){                                     //Percorre as colunas
    for (int i = 0; i < img->width; i++){                                    //Percorre as linhas
      ImageSetPixel(imgMirror,i, j,ImageGetPixel(img,(img->width-1)-i,j));      //Atribui ao pixel (x,y) da imagem espelhada o valor do pixel (largura-x, y) da imagem 1 de forma a espelhar a imagem horizontalmente da esquerda para a direita
    }
  }
  return imgMirror; //Retorna a imagem espelhada
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) {                              //---------------- Função escrita dia 18/11/2023
  assert (img != NULL);
  assert (ImageValidRect(img, x, y, w, h));                       //Verifica se o recangulo de largura w e altura h está dentro da iamgem 1
  Image imgCrop = ImageCreate(w,h,img->maxval);                   //Cria uma nova imagem, a imagem cortada
  for (int j = y; j < y+h; j++){                                  //Percorre as colunas
    for (int i = x; i < x+w; i++){                                //Percorre as linhas
      ImageSetPixel(imgCrop,i-x,j-y,ImageGetPixel(img, i, j));    //atribui à nova imagem os valores correspodentes aos pixies da imagem 1 que estão dentro do retangulo w*h
    } 
  }
  return imgCrop;   //Retorna a imagem cortada
}


/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) {                  //---------------- Função escrita dia 18/11/2023
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));     //Verifica se a imagem 2 está dentro da imagem 1
  for (int j = y; j < y+img2->height; j++){                 //Percorre as colunas
    for (int i = x; i < x+img2->width; i++){                //Percorre as linhas
      ImageSetPixel(img1, i, j,ImageGetPixel(img2,i-x, j-y));     //Substitui os pixeis da imagem 1 pelos da iamgem 2 no retangulo com o tamanho da imagem 2 e origem e (x,y)
    }
  }  
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));     //Verifica se a iamgem 2 está dentro da imagem 1
  for (int j = y; j < y+img2->height; j++){           //Percorre as colunas
    for (int i = x; i < x+img2->width; i++){          //Percorre as linhas
      double img1Pixel = ImageGetPixel(img1, i, j); //Armazena o pixel da imagem 1
      double img2Pixel = ImageGetPixel(img2, i-x, j-y); //Armazena o pixel da imagem 2
      double blendPixel = (int)((1.0-alpha)*img1Pixel + img2Pixel*alpha + 0.5); //Faz o blend dos pixeis de ambas as imagens
      ImageSetPixel(img1, i, j, blendPixel);     //Substitui os pixeis da imagem 1 pelos "blendPixel" no retangulo com o tamanho da imagem 2 e origem e (x,y)
    }
  }  
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) {                            //---------------- Função escrita dia 18/11/2023
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidPos(img1, x, y));                       //Verifica se o pizel na posição (x,y) está dentro da iamgem 1       
  for (int j = y; j < y+img2->height; j++){            //Percorre as colunas
    for (int i = x; i < x+img2->width; i++){           //Percorre as linhas
      double img1Pixel = ImageGetPixel(img1, i, j); //Armazena o pixel da imagem 1
      double img2Pixel = ImageGetPixel(img2, i-x, j-y); //Armazena o pixel da imagem 2
      LocateCompar++;     // variável contadora para incrementar sempre que compare 
      if (img1Pixel!=img2Pixel){  //Verifica se os pixeis são diferentes
        return 0;                 //Se os pixeis forem diferentes retorna 0
      }
    }
  }
  return 1;                       //Se todos os pixeis forem iguais retorna 1 (após percorrer todos os pixeis da imagem 2 e os pixeis pretendidos da iamgem 1)
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2) {                         //---------------- Função escrita dia 19/11/2023
  assert (img1 != NULL);
  assert (img2 != NULL);

  for (int j = 0; j <= img1->height - img2->height; j++){               //Percorre as colunas
    for (int i = 0; i <= img1->width - img2->width; i++){               //Percorre as linhas
      LocateCompar++;                          // Variável contadora para numero de comparações
      if (ImageMatchSubImage(img1, i, j, img2)){      //Se os pixeis coincidirem:                              
        *px = i;                                      //Atribui as coordenadas x a *px
        *py = j;                                      //Atribui as coordenadas y a *py                               
        return 1;                                     //Retorna 1;
      }
    }
  } 
  return 0;                                           //Caso não haja uma subimagem 2 na imagem 1 retorna 0;
}


/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.

void ImageBlur(Image img, int dx, int dy) {                                            //---------------- Função escrita dia 19/11/2023
  assert(img != NULL);
  double sum;                 //Variável que vai somar o valor dos pixeis da imagem 2
  int cont;                   //Contador
  Image img2 = ImageCreate(img->width, img->height, img->maxval);     //Cria uma nova imagem igual à primeira onde se irá buscar o valor dos pixeis uma vez que os da imagem 1 serão alterados
  
  
  for (long i = 0; i < img->width*img->height; i++){
    img2->pixel[i]=img->pixel[i];                                     //copia os valores dos pixeis da imagem 1 para os da imagem 2
  }
  
  for (int j = 0; j < img->height; j++){              //Percorre as colunas
    for (int i = 0; i < img->width; i++){             //Percorre as linhas
      sum = 0;                                        //Reinicia o sum
      cont = 0;                                       //Reinicia o contador
      for (int k = j-dy; k <= j+dy; k++){
        if (k<0 || k >= img->height){                    //Verifica se a coordenada x da imagem 2 está dentro da imagem 1
          continue;                                   //Se sim continua
        }
        
        for (int t = i-dx; t <= i+dx; t++){  
          if (t<0 || t>=img->width){               //Verifica se a coordenada y da imagem 2 está dentro da imagem 1
            continue;                                 //Se sim continua
          }
          
          sum += ImageGetPixel(img2, t, k);           //Soma o valor do pixel da imagem 2 na coordenadas (t,k)
          cont++;                                     //incrementa 1 valor ao contador
        }
      }
      int meanFilter =(int)((sum/cont) + 0.5);        // Cria o meanFilter e atribui-lhe o devido valor
      ImageSetPixel(img, i, j, meanFilter);           //Substitui o pixel original da imagem 1 pelo pixel com o novo valor depois de aplicado o meanFilter
    }
  }
    ImageDestroy(&img2);                              //Destrói a imagem 2
}

void ImageBlurMelhorado(Image img, int dx, int dy) {         ///Versão melhorada da função Blur ---- Escrita dia 24
   assert(img != NULL);

   uint8* original = malloc(img->width * img->height * sizeof(uint8));        // Aloca memória para uma cópia da imagem original
   memcpy(original, img->pixel, img->width * img->height * sizeof(uint8));    // copia os dados dos pixeis para a variável original --- memcpy(void *to, const void *from, size_t numBytes);
   for (int j = 0; j < img->height; j++) {          //Percorre as colunas
       for (int i = 0; i < img->width; i++) {       //Percorre as linhas
           double sum = 0;
           int count = 0;
           for (int k = -dy; k <= dy; k++) {
               for (int t = -dx; t <= dx; t++) {
                if ((i+t >= 0 && i+t < img->width) && (j+k >= 0 && j+k < img->height)){
                  sum += original[(int)((j + k) * img->width) + (int)(i + t)];        //Soma o valor dos pixeis na coordenadas (t,k)
                  count++;                                                //incrementa 1 valor ao contador
                }
               }
           }
           img->pixel[j * img->width + i] = (int)((sum/count)+0.5);
       }
   }

   free(original); //Liberta a memória alocada para a imagem original
}


