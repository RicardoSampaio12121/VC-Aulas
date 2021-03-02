#include <stdio.h>
#include "vc.h"

int main(void){
    IVC *src, *dst;
    int x, y;
    long int pos;

    src = vc_read_image("Images/Classic/pepper.ppm");
    dst = vc_image_new(src->width, src->height, 1, src->levels);

    if(src == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    
    vc_rgb_to_gray(src, dst);

    vc_write_image("goldhill.pgm", dst);

    vc_image_free(src);
    vc_image_free(dst);


    printf("Press any key to exit...\n");
    getchar();

    return 0;
}