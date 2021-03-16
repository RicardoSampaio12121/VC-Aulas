#include <stdio.h>
#include "vc.h"
#include <string.h>

int main(void){
    IVC *src, *dst, *dstt, *dsttt;
    int x, y;
    long int pos;

    src = vc_read_image("Images/FLIR/flir-01.pgm");
    dst = vc_image_new(src->width, src->height, 1, 1);
    dstt = vc_image_new(src->width, src->height, 1, 1);
    dsttt = vc_image_new(src->width, src->height, 1, 1);

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

    
   
    vc_gray_to_binary(src, dst, 120);
    vc_binary_erode(dst, dstt, 3);
    vc_binary_dilate(dstt, dsttt, 3);

    vc_write_image("teste1.pbm", dst);
    vc_write_image("teste2.pbm", dstt);
    vc_write_image("teste3.pbm", dsttt);


    vc_image_free(src);
    vc_image_free(dst);
    vc_image_free(dstt);
    vc_image_free(dsttt);



    printf("Press any key to exit...\n");
    getchar();

    return 0;
}




