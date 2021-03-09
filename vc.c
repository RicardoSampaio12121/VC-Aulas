//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
    IVC *image = (IVC *) malloc(sizeof(IVC));

    if(image == NULL) return NULL;
    if((levels <= 0) || (levels > 255)) return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = image->width * image->channels;
    image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

    if(image->data == NULL)
    {
        return vc_image_free(image);
    }

    return image;
}


// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
    if(image != NULL)
    {
        if(image->data != NULL)
        {
            free(image->data);
            image->data = NULL;
        }

        free(image);
        image = NULL;
    }

    return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
    char *t;
    int c;

    for(;;)
    {
        while(isspace(c = getc(file)));
        if(c != '#') break;
        do c = getc(file);
        while((c != '\n') && (c != EOF));
        if(c == EOF) break;
    }

    t = tok;

    if(c != EOF)
    {
        do
        {
            *t++ = c;
            c = getc(file);
        } while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

        if(c == '#') ungetc(c, file);
    }

    *t = 0;

    return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
    int x, y;
    int countbits;
    long int pos, counttotalbytes;
    unsigned char *p = databit;

    *p = 0;
    countbits = 1;
    counttotalbytes = 0;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pos = width * y + x;

            if(countbits <= 8)
            {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //*p |= (datauchar[pos] != 0) << (8 - countbits);

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                *p |= (datauchar[pos] == 0) << (8 - countbits);

                countbits++;
            }
            if((countbits > 8) || (x == width - 1))
            {
                p++;
                *p = 0;
                countbits = 1;
                counttotalbytes++;
            }
        }
    }

    return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
    int x, y;
    int countbits;
    long int pos;
    unsigned char *p = databit;

    countbits = 1;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pos = width * y + x;

            if(countbits <= 8)
            {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

                countbits++;
            }
            if((countbits > 8) || (x == width - 1))
            {
                p++;
                countbits = 1;
            }
        }
    }
}


IVC *vc_read_image(char *filename)
{
    FILE *file = NULL;
    IVC *image = NULL;
    unsigned char *tmp;
    char tok[20];
    long int size, sizeofbinarydata;
    int width, height, channels;
    int levels = 255;
    int v;

    // Abre o ficheiro
    if((file = fopen(filename, "rb")) != NULL)
    {
        // Efectua a leitura do header
        netpbm_get_token(file, tok, sizeof(tok));

        if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
        else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
        else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
        else
        {
#ifdef VC_DEBUG
            printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

            fclose(file);
            return NULL;
        }

        if(levels == 1) // PBM
        {
            if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
               sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca mem�ria para imagem
            image = vc_image_new(width, height, channels, levels);
            if(image == NULL) return NULL;

            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
            tmp = (unsigned char *) malloc(sizeofbinarydata);
            if(tmp == NULL) return 0;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                free(tmp);
                return NULL;
            }

            bit_to_unsigned_char(tmp, image->data, image->width, image->height);

            free(tmp);
        }
        else // PGM ou PPM
        {
            if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
               sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
               sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca mem�ria para imagem
            image = vc_image_new(width, height, channels, levels);
            if(image == NULL) return NULL;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            size = image->width * image->height * image->channels;

            if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                return NULL;
            }
        }

        fclose(file);
    }
    else
    {
#ifdef VC_DEBUG
        printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
    }

    return image;
}


int vc_write_image(char *filename, IVC *image)
{
    FILE *file = NULL;
    unsigned char *tmp;
    long int totalbytes, sizeofbinarydata;

    if(image == NULL) return 0;

    if((file = fopen(filename, "wb")) != NULL)
    {
        if(image->levels == 1)
        {
            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
            tmp = (unsigned char *) malloc(sizeofbinarydata);
            if(tmp == NULL) return 0;

            fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

            totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
            printf("Total = %ld\n", totalbytes);
            if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
            {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                free(tmp);
                return 0;
            }

            free(tmp);
        }
        else
        {
            fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

            if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
            {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                return 0;
            }
        }

        fclose(file);

        return 1;
    }

    return 0;
}

int vc_gray_negative(IVC *srcdst)
{
    unsigned char *data = (unsigned char *) srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int byterperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    //verificação de erros
    if(width <= 0 || height <= 0 || data == NULL) return 0;
    if(channels != 1) return 0;

    //Invert an image gray
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            pos = y * byterperline + x * channels;
            data[pos] = (unsigned char)(255 - data[pos]);
        }
    } 

    return 1;
}

/* 
int vc_rgb_negative(IVC *srcdst)
{
    unsigned char *data = (unsigned char *) srcdst->data;
    int width =srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long ins pos;

    if(width <= 0 || hei\ <= 0)
} */

int vc_rgb_get_red_gray(IVC *srcdst)
{
    unsigned char *data = (unsigned char *) srcdst->data;
    int width =srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    //verificação de erros

    if((width <= 0) || (height <= 0) ||(data == NULL)) return 0;
    if(channels != 3) return 0;

    //Extrai o componente Red
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos + 1] = data[pos];
            data[pos + 2] = data[pos];
        }
    }
}

int vc_rgb_get_green_gray(IVC *srcdst)
{
    unsigned char *data = (unsigned char *) srcdst->data;
    int width =srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    //verificação de erros

    if((width <= 0) || (height <= 0) ||(data == NULL)) return 0;
    if(channels != 3) return 0;

    //Extrai o componente Red
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos] = data[pos + 1];
            data[pos + 2] = data[pos + 1];
        }
    }
}

int vc_rgb_get_blue_gray(IVC *srcdst)
{
    unsigned char *data = (unsigned char *) srcdst->data;
    int width =srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    //verificação de erros

    if((width <= 0) || (height <= 0) ||(data == NULL)) return 0;
    if(channels != 3) return 0;

    //Extrai o componente Red
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos] = data[pos + 2];
            data[pos + 1] = data[pos + 2];
        }
    }
}

int vc_rgb_to_gray(IVC *src, IVC *dst)
{
    unsigned char *datasrc = (unsigned char *) src->data;
    int byterperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst = (unsigned char *) dst->data;
    int byterperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src, pos_dst;
    float rf, gf, bf;

    if((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height)) return 0;
    if((src->channels != 3) || (dst->channels != 1)) return 0;

    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            pos_src = y * byterperline_src + x * channels_src;
            pos_dst = y * byterperline_dst + x * channels_dst;

            rf = (float) datasrc[pos_src];
            gf = (float) datasrc[pos_src + 1];
            bf = (float) datasrc[pos_src + 2];

            datadst[pos_dst] = (unsigned char) ((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
        }
    }
    return 1;
}

int vc_rgb_to_hsv(IVC *src, IVC *dst)
{
    unsigned char *data = (unsigned char *) src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	size = width * height * channels;
	
	for(i=0; i<size; i+=channels)
	{
       
		r = (float) data[i];
		g = (float) data[i + 1];
		b = (float) data[i + 2];
		
		// Calcula valores m�ximo e m�nimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));
		
		// Value toma valores entre [0, 255]
		value = rgb_max;
		if(value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			// Saturation toma valores entre [0, 1]
			saturation = ((rgb_max - rgb_min) / rgb_max);

			if(saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				// Hue toma valores entre [0, 360]
				if((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if(rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else /* rgb_max == b*/
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		// Atribui valores entre [0, 255]
		dst->data[i]     = (unsigned char) (hue / 360.0f * 255.0f);
		dst->data[i + 1] = (unsigned char) (saturation * 255.0f);
		dst->data[i + 2] = (unsigned char) (value);
	}
	return 1;
}


int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
    int size = src->width * src->height * src->channels;
    int value;
    int z = 0;

    for(int i = 0; i < size; i+=src->channels)
    {
        float h = ((float)src->data[i] / 255.0f) * 360.0f;
        float s = (src->data[i+1] / 255.0f) * 100.0f;
        float v = (src->data[i+2] / 255.0f) * 100.0f; 
    

        if(h >= hmin && h <= hmax && s >= smin && s <= smax && v >= vmin && v <= vmax) 
        {
            dst->data[z] = (unsigned char) 255;
        } 
        else
        {
            dst->data[z] = (unsigned char) 0;
        }
        z++;
    }
}

//Recebe uma imagem com 1 canal
//Sai uma imagem com 3 canais
int vc_scale_gray_to_rgb(IVC *src, IVC *dst)
{
    int size = src->width * src->height * dst->channels;
    int z = 0;
    int r, g, b;

    for(int i = 0; i < size; i += dst->channels)
    {
        if(src->data[z] < 64)
        {
            r = 0;
            g = src->data[z] * 4;
            b = 255;
        }
        else if(src->data[z] >= 64 && src->data[z] < 128)
        {
            r = 0;
            g = 255;
            b = 255 - (src->data[z] - 64) * 4;
        }
        else if(src->data[z] >= 128 && src->data[z] < 192)
        {
            r = 255 - (src->data[z] + 128) * 4;
            g = 255;
            b = 0;
        }
        else
        {
            r = 255;
            g = 255 - (src->data[z] - 192) * 4;
            b = 0;
        }

        dst->data[i] = (unsigned char)r;
        dst->data[i+1] = (unsigned char)g;
        dst->data[i+2] = (unsigned char)b;

        z++;
    }
}
