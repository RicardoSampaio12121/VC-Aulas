#include <stdio.h>
#include "vc.h"
#include <string.h>

int main(void){
    IVC *image[3];
    int x, y;
    long int pos;
    image[0] = vc_read_image("Images/barbara_heq.pgm");
    image[1] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);

    

    if(image[0] == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    if(image[1] == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }
   
    vc_gray_histogram_equalization(image[0], image[1]);


    vc_write_image("barbara_heq.pgm", image[1]); 
    
    vc_image_free(image[0]);
    vc_image_free(image[1]);
    //vc_image_free(image[2]);

   

 /* vc_binary_dilate(dst, src, 3);
    vc_binary_erode(src, dst, 3); */

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}




