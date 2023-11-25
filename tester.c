#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image8bit.h"


int test1(){
    ImageInit();
    Image img = ImageLoad("bri.pgm");
    Image img2 = ImageLoad("bricopy.pgm");
    Image img3 = ImageLoad("original.pgm");
    uint8 x;
    uint8 x1;
    int o;
    for(long u=0;u<ImageHeight(img);u++){
        for (long y=0;y<ImageWidth(img);y++) {
            if(!(ImageGetPixel(img, y,u) == ImageGetPixel(img2,y,u))){
                printf("%d|%d||1%d|2%d/o%d\n",u,y,ImageGetPixel(img, y,u),
                ImageGetPixel(img2,y,u),ImageGetPixel(img3, y,u));
            }
        }
    }
    printf("%d",(uint8)(ImageGetPixel(img3, 150,2)*0.33));
    printf("\\%d",(ImageGetPixel(img, 150,2))==ImageGetPixel(img2, 150,2));
    uint8 max,min;
    printf("Ini");
    ImageStats(img2, &min, &max);
    printf("\n%d//%d",max,min);
    ImageDestroy(&img);
    ImageDestroy(&img2);
    ImageDestroy(&img3);
    printf("f");
    return NULL;
}
int test2(){
    ImageInit();
    Image img = ImageLoad("rotate.pgm");
    Image img2 = ImageLoad("rotate copy.pgm");
    uint8 x;
    uint8 x1;
    int o;
    for(long u=0;u<ImageHeight(img);u++){
        for (long y=0;y<ImageWidth(img);y++) {
            if(!(ImageGetPixel(img, y,u) == ImageGetPixel(img2,y,u))){
                printf("%d|%d||R%d|C%d\n",u,y,ImageGetPixel(img, y,u),
                ImageGetPixel(img2,y,u));
            }
        }
    }
    ImageDestroy(&img);
    ImageDestroy(&img2);
    printf("f");
    return NULL;
}

int test3(){
    ImageInit();
    Image img = ImageLoad("blend.pgm");
    Image img2 = ImageLoad("blend copy.pgm");
    uint8 x;
    uint8 x1;
    int o;
    for(long u=0;u<ImageHeight(img);u++){
        for (long y=0;y<ImageWidth(img);y++) {
            if(!(ImageGetPixel(img, y,u) == ImageGetPixel(img2,y,u))){
                printf("%d|%d||R%d|C%d\n",u,y,ImageGetPixel(img, y,u),
                ImageGetPixel(img2,y,u));
            }
        }
    }
    ImageDestroy(&img);
    ImageDestroy(&img2);
    printf("f");
    return NULL;
}
int test4(){
    ImageInit();
    Image img = ImageLoad("blur.pgm");
    Image img2 = ImageLoad("test/blur.pgm");
    uint8 x;
    uint8 x1;
    int o;
    for(long u=0;u<ImageHeight(img);u++){
        for (long y=0;y<ImageWidth(img);y++) {
            if(!(ImageGetPixel(img, y,u) == ImageGetPixel(img2,y,u))){
                printf("%d|%d||R%d|C%d\n",u,y,ImageGetPixel(img, y,u),
                ImageGetPixel(img2,y,u));
            }
        }
    }
    ImageDestroy(&img);
    ImageDestroy(&img2);
    printf("f");
    return NULL;
}

int main(){
    test4();
}
//bri copy
//printf("%d|%d||%d|%d||%d\n",y,imgInWidth-x,x,y,ImageValidPos(imgReturn,imgInHEight-y-1, imgInWidth-x-1));
//blend copy