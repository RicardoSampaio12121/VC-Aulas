#include <stdio.h>
#include "vc.h"
#include <string.h>

int main(void){
    IVC *src, *dst, *dstt;
    int x, y;
    long int pos;

    src = vc_read_image("Images/HSV/HSVTestImage01.ppm");
    dst = vc_image_new(src->width, src->height, 3, src->levels);
    dstt = vc_image_new(src->width, src->height, 1, src->levels);

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

    
    vc_rgb_to_hsv(src, dst);
    
    printf("esta aqui\n");
    vc_hsv_segmentation(dst, dstt, 30, 70, 50, 100, 60, 100);



    vc_write_image("teste.ppm", dstt);


    vc_image_free(src);
    vc_image_free(dst);


    printf("Press any key to exit...\n");
    getchar();

    return 0;
}



