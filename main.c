#include <stdio.h>
#include "vc.h"
#include <string.h>

int main(void){
    IVC *src, *dst, *dstt;
    int x, y;
    long int pos;

    src = vc_read_image("Images/FLIR/flir-04.pgm");
    dst = vc_image_new(src->width, src->height, 3, src->levels);
   

    if(src == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    if(dst == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    
   
    vc_scale_gray_to_rgb(src, dst);

    vc_write_image("teste.ppm", dst);


    vc_image_free(src);
    vc_image_free(dst);


    printf("Press any key to exit...\n");
    getchar();

    return 0;
}



