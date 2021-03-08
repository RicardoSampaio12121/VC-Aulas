#include <stdio.h>
#include "vc.h"
#include <string.h>

int main(void){
    IVC *src, *dst;
    int x, y;
    long int pos;

    src = vc_read_image("Images/HSV/HSVTestImage01.ppm");
    dst = vc_image_new(src->width, src->height, src->channels, src->levels);

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

    vc_write_image("goldhill.ppm", dst);

    vc_image_free(src);
    vc_image_free(dst);


    printf("Press any key to exit...\n");
    getchar();

    return 0;
}