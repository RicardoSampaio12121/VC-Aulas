#include <stdio.h>
#include "vc.h"
#include <string.h>

int main(void){
    IVC *image[3];
    int x, y;
    long int pos;
    image[0] = vc_read_image("Images/labelling-2.pgm");
    image[1] = vc_image_new(image[0]->width, image[0]->height, 1, 1);
    image[2] = vc_image_new(image[0]->width, image[0]->height, 1, 1);

    

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

    if(image[2] == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    
   
    //vc_gray_to_binary(image[0], image[1], 120);
    
    vc_binary_blob_labelling(image[0], image[1]);


    vc_write_image("openClose.pbm", image[1]); 
    
    vc_image_free(image[0]);
    vc_image_free(image[1]);
    //vc_image_free(image[2]);

   

 /* vc_binary_dilate(dst, src, 3);
    vc_binary_erode(src, dst, 3); */

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}




