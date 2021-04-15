/*
    Author: Ricardo Sampaio
    Author Cláudio Silva
    Date: 14/04/2021
*/
#include <stdio.h>
#include "vc.h"


int main()
{
    IVC *image[7];
    IVC *etiquetagemOriginal;
    OVC *info;

    int nlabels;

    image[0] = vc_read_image("cerebro.pgm");
    image[1] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels);
    image[2] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels); 
    image[3] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels); 
    image[4] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels); 
    image[5] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels); 
    image[6] = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels); 
    etiquetagemOriginal = vc_image_new(image[0]->width, image[0]->height, image[0]->channels, image[0]->levels); 




    //Primeiro passar a imagem para binária
    //vc_gray_to_binary_midpoint(image[0], image[1], 10);
    //vc_gray_to_binary_global_mean(image[0], image[1]);
    //vc_gray_to_binary(image[0], image[1], 100);
    
    vc_gray_to_binary_max_min(image[0], image[1], 74, 199);

    //Eliminar a parte do cranio da imagem
    //Primeiro dilatamos a imagem para juntar as partes do cerebro
    /* 
    */
    vc_binary_erode(image[1], image[2], 7);
    vc_binary_dilate(image[2], image[3], 7); 
    vc_binary_close(image[3], image[4], 7);

    info = vc_binary_blob_labelling(image[4], image[5], &nlabels);

    if (info != NULL)
        vc_binary_blob_info(image[5], info, nlabels);
    

    vc_gray_label(image[0], image[4]);
    
    
    /*
    printf("%d\n", nlabels); */
    
    vc_write_image("segmentation.pgm" ,image[1]);
    vc_write_image("erode.pgm" ,image[2]);
    vc_write_image("dilate.pgm" ,image[3]);
    vc_write_image("close.pgm", image[4]);
    vc_write_image("labeled.pgm", image[5]);


    
    
    vc_image_free(image[0]);
    vc_image_free(image[1]);
    vc_image_free(image[2]);
    vc_image_free(image[3]);
/*     vc_image_free(image[4]); */




    
    return 0;
}
