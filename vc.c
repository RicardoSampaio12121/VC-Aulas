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
#include <math.h>
#include "vc.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
    IVC *image = (IVC *)malloc(sizeof(IVC));

    if (image == NULL)
        return NULL;
    if ((levels <= 0) || (levels > 255))
        return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = image->width * image->channels;
    image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

    if (image->data == NULL)
    {
        return vc_image_free(image);
    }

    return image;
}

// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
    if (image != NULL)
    {
        if (image->data != NULL)
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

    for (;;)
    {
        while (isspace(c = getc(file)))
            ;
        if (c != '#')
            break;
        do
            c = getc(file);
        while ((c != '\n') && (c != EOF));
        if (c == EOF)
            break;
    }

    t = tok;

    if (c != EOF)
    {
        do
        {
            *t++ = c;
            c = getc(file);
        } while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

        if (c == '#')
            ungetc(c, file);
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

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = width * y + x;

            if (countbits <= 8)
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
            if ((countbits > 8) || (x == width - 1))
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

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = width * y + x;

            if (countbits <= 8)
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
            if ((countbits > 8) || (x == width - 1))
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
    if ((file = fopen(filename, "rb")) != NULL)
    {
        // Efectua a leitura do header
        netpbm_get_token(file, tok, sizeof(tok));

        if (strcmp(tok, "P4") == 0)
        {
            channels = 1;
            levels = 1;
        } // Se PBM (Binary [0,1])
        else if (strcmp(tok, "P5") == 0)
            channels = 1; // Se PGM (Gray [0,MAX(level,255)])
        else if (strcmp(tok, "P6") == 0)
            channels = 3; // Se PPM (RGB [0,MAX(level,255)])
        else
        {
#ifdef VC_DEBUG
            printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

            fclose(file);
            return NULL;
        }

        if (levels == 1) // PBM
        {
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
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
            if (image == NULL)
                return NULL;

            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
            tmp = (unsigned char *)malloc(sizeofbinarydata);
            if (tmp == NULL)
                return 0;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
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
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
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
            if (image == NULL)
                return NULL;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            size = image->width * image->height * image->channels;

            if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
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

    if (image == NULL)
        return 0;

    if ((file = fopen(filename, "wb")) != NULL)
    {
        if (image->levels == 1)
        {
            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
            tmp = (unsigned char *)malloc(sizeofbinarydata);
            if (tmp == NULL)
                return 0;

            fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

            totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
            printf("Total = %ld\n", totalbytes);
            if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
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

            if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
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
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int byterperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    //verificação de erros
    if (width <= 0 || height <= 0 || data == NULL)
        return 0;
    if (channels != 1)
        return 0;

    //Invert an image gray
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
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
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    //verificação de erros

    if ((width <= 0) || (height <= 0) || (data == NULL))
        return 0;
    if (channels != 3)
        return 0;

    //Extrai o componente Red
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos + 1] = data[pos];
            data[pos + 2] = data[pos];
        }
    }
}

int vc_rgb_get_green_gray(IVC *srcdst)
{
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    //verificação de erros

    if ((width <= 0) || (height <= 0) || (data == NULL))
        return 0;
    if (channels != 3)
        return 0;

    //Extrai o componente Red
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos] = data[pos + 1];
            data[pos + 2] = data[pos + 1];
        }
    }
}

int vc_rgb_get_blue_gray(IVC *srcdst)
{
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->bytesperline;
    int channels = srcdst->channels;
    int x, y;
    long int pos;

    //verificação de erros

    if ((width <= 0) || (height <= 0) || (data == NULL))
        return 0;
    if (channels != 3)
        return 0;

    //Extrai o componente Red
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            data[pos] = data[pos + 2];
            data[pos + 1] = data[pos + 2];
        }
    }
}

int vc_rgb_to_gray(IVC *src, IVC *dst)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    int byterperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst = (unsigned char *)dst->data;
    int byterperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src, pos_dst;
    float rf, gf, bf;

    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
        return 0;
    if ((src->width != dst->width) || (src->height != dst->height))
        return 0;
    if ((src->channels != 3) || (dst->channels != 1))
        return 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos_src = y * byterperline_src + x * channels_src;
            pos_dst = y * byterperline_dst + x * channels_dst;

            rf = (float)datasrc[pos_src];
            gf = (float)datasrc[pos_src + 1];
            bf = (float)datasrc[pos_src + 2];

            datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
        }
    }
    return 1;
}

int vc_rgb_to_hsv(IVC *src, IVC *dst)
{
    unsigned char *data = (unsigned char *)src->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    float r, g, b, hue, saturation, value;
    float rgb_max, rgb_min;
    int i, size;

    size = width * height * channels;

    for (i = 0; i < size; i += channels)
    {

        r = (float)data[i];
        g = (float)data[i + 1];
        b = (float)data[i + 2];

        // Calcula valores m�ximo e m�nimo dos canais de cor R, G e B
        rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
        rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

        // Value toma valores entre [0, 255]
        value = rgb_max;
        if (value == 0.0f)
        {
            hue = 0.0f;
            saturation = 0.0f;
        }
        else
        {
            // Saturation toma valores entre [0, 1]
            saturation = ((rgb_max - rgb_min) / rgb_max);

            if (saturation == 0.0f)
            {
                hue = 0.0f;
            }
            else
            {
                // Hue toma valores entre [0, 360]
                if ((rgb_max == r) && (g >= b))
                {
                    hue = 60.0f * (g - b) / (rgb_max - rgb_min);
                }
                else if ((rgb_max == r) && (b > g))
                {
                    hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
                }
                else if (rgb_max == g)
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
        dst->data[i] = (unsigned char)(hue / 360.0f * 255.0f);
        dst->data[i + 1] = (unsigned char)(saturation * 255.0f);
        dst->data[i + 2] = (unsigned char)(value);
    }
    return 1;
}

int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
    int size = src->width * src->height * src->channels;
    int value;
    int z = 0;

    for (int i = 0; i < size; i += src->channels)
    {
        float h = ((float)src->data[i] / 255.0f) * 360.0f;
        float s = (src->data[i + 1] / 255.0f) * 100.0f;
        float v = (src->data[i + 2] / 255.0f) * 100.0f;

        if (h >= hmin && h <= hmax && s >= smin && s <= smax && v >= vmin && v <= vmax)
        {
            dst->data[z] = (unsigned char)255;
        }
        else
        {
            dst->data[z] = (unsigned char)0;
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

    for (int i = 0; i < size; i += dst->channels)
    {
        if (src->data[z] < 64)
        {
            r = 0;
            g = src->data[z] * 4;
            b = 255;
        }
        else if (src->data[z] >= 64 && src->data[z] < 128)
        {
            r = 0;
            g = 255;
            b = 255 - (src->data[z] - 64) * 4;
        }
        else if (src->data[z] >= 128 && src->data[z] < 192)
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
        dst->data[i + 1] = (unsigned char)g;
        dst->data[i + 2] = (unsigned char)b;

        z++;
    }
}

int vc_gray_to_binary(IVC *src, IVC *dst, int threshold)
{
    int size = src->height * src->width;

    for (int i = 0; i < size; i++)
    {
        if (src->data[i] > threshold)
        {
            dst->data[i] = 255;
        }
        else
        {
            dst->data[i] = 0;
        }
    }
}

int vc_gray_to_binary_max_min(IVC *src, IVC *dst, int minThreshold, int maxThreshold)
{
    int size = src->height * src->width;

    for (int i = 0; i < size; i++)
    {
        if (src->data[i] > maxThreshold || src->data[i] < minThreshold)
        {
            dst->data[i] = 0;
        }
        else
        {
            dst->data[i] = 255;
        }
    }
}

int vc_gray_to_binary_global_mean(IVC *src, IVC *dst)
{
    int size = src->height * src->width;
    int count = 0;

    for (int i = 0; i < size; i++)
    {
        count += (int)src->data[i];
    }

    int threshold = count / size;

    printf("%d\n", threshold);
    getchar();

    vc_gray_to_binary(src, dst, threshold);
}

int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y, kx, ky;
    int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
    int max, min;
    long int pos, posk;
    unsigned char threshold;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
        return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
        return 0;
    if (channels != 1)
        return 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            max = 0;
            min = 255;

            // NxM Vizinhos
            for (ky = -offset; ky <= offset; ky++)
            {
                for (kx = -offset; kx <= offset; kx++)
                {
                    if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
                    {
                        posk = (y + ky) * bytesperline + (x + kx) * channels;

                        if (datasrc[posk] > max)
                            max = datasrc[posk];
                        if (datasrc[posk] < min)
                            min = datasrc[posk];
                    }
                }
            }

            threshold = (unsigned char)((float)(max + min) / (float)2);

            if (datasrc[pos] > threshold)
                datadst[pos] = 255;
            else
                datadst[pos] = 0;
        }
    }

    return 1;
}

int vc_binary_dilate(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y, kx, ky;
    int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
    int max, min;
    long int pos, posk;
    unsigned char threshold;
    int isT = 0;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
        return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
        return 0;
    if (channels != 1)
        return 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            max = 0;
            min = 255;

            isT = 0;
            // NxM Vizinhos
            for (ky = -offset; ky <= offset; ky++)
            {
                for (kx = -offset; kx <= offset; kx++)
                {
                    if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
                    {
                        posk = (y + ky) * bytesperline + (x + kx) * channels;
                        if (datasrc[posk] == 255)
                            isT = 1;
                    }
                }
            }

            threshold = (unsigned char)((float)(max + min) / (float)2);

            if (isT == 1)
                datadst[pos] = 255;
            else
                datadst[pos] = 0;
        }
    }

    return 1;
}

int vc_binary_erode(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y, kx, ky;
    int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
    int max, min;
    long int pos, posk;
    unsigned char threshold;
    int isT = 0;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
        return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
        return 0;
    if (channels != 1)
        return 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;

            max = 0;
            min = 255;

            isT = 0;
            // NxM Vizinhos
            for (ky = -offset; ky <= offset; ky++)
            {
                for (kx = -offset; kx <= offset; kx++)
                {
                    if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
                    {
                        posk = (y + ky) * bytesperline + (x + kx) * channels;
                        if (datasrc[posk] == 0)
                            isT = 1;
                    }
                }
            }

            threshold = (unsigned char)((float)(max + min) / (float)2);

            if (isT == 1)
                datadst[pos] = 0;
            else
                datadst[pos] = 255;
        }
    }

    return 1;
}

int vc_binary_open(IVC *src, IVC *dst, int kernel)
{
    IVC *tmp = vc_image_new(src->width, src->height, src->channels, src->levels);

    vc_binary_erode(src, tmp, kernel);
    vc_binary_dilate(tmp, dst, kernel);
}

int vc_binary_close(IVC *src, IVC *dst, int kernel)
{
    IVC *tmp = vc_image_new(src->width, src->height, src->channels, src->levels);

    vc_binary_dilate(src, tmp, kernel);
    vc_binary_erode(tmp, dst, kernel);
}

/* int vc_binary_blob_labelling(IVC *src, IVC *dst)
{
    int label = 1;
    int pos_src, pos_dst;
    int pos_a = 0, pos_b = 0, pos_c = 0, pos_d = 0;
    int min = 0;

    for (int y = 0; y < src->height; y++)
    {
        for (int x = 0; x < src->width; x++)
        {
            pos_src = y * src->bytesperline + x * src->channels;
            pos_dst = y * dst->bytesperline + x * dst->channels;

            if (src->data[pos_src] == 255)
            {
                pos_a = pos_src - src->width - 1;
                pos_b = pos_src - src->width;
                pos_c = pos_src - src->width + 1;
                pos_d = pos_src - 1;

                if (src->data[pos_a] == 0 && src->data[pos_b] == 0 && src->data[pos_c] == 0 && src->data[pos_d] == 0)
                {
                    dst->data[pos_src] = label;
                    label++;
                }
                else
                {
                    int minLabel = 0;
                    if (dst->data[pos_a] <= dst->data[pos_b] && dst->data[pos_a] <= dst->data[pos_c] && dst->data[pos_a] <= dst->data[pos_d] && dst->data[pos_a] != 0)
                    {
                        minLabel = dst->data[pos_a];
                    }
                    else if (dst->data[pos_b] <= dst->data[pos_a] && dst->data[pos_b] <= dst->data[pos_c] && dst->data[pos_b] <= dst->data[pos_d] && dst->data[pos_b] != 0)
                    {
                        minLabel = dst->data[pos_b];
                    }
                    else if (dst->data[pos_c] <= dst->data[pos_b] && dst->data[pos_c] <= dst->data[pos_a] && dst->data[pos_c] <= dst->data[pos_d] && dst->data[pos_c] != 0)
                    {
                        minLabel = dst->data[pos_c];
                    }
                    else if (dst->data[pos_d] <= dst->data[pos_b] && dst->data[pos_d] <= dst->data[pos_a] && dst->data[pos_d] <= dst->data[pos_c] && dst->data[pos_d] != 0)
                    {
                        minLabel = dst->data[pos_d];
                    }

                    dst->data[pos_src] = minLabel;
                }
            }
            else
            {
                dst->data[pos_src] = 0;
            }
        }
    }

    printf("Label: %d\n", label);
} */

// Etiquetagem de blobs
// src		: Imagem binária de entrada
// dst		: Imagem grayscale (irá conter as etiquetas)
// nlabels	: Endereço de memória de uma variável, onde será armazenado o número de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. É necessário libertar posteriormente esta memória.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;
   
	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

    

	// Conta área de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

int teste(IVC* src, IVC* dst)
{
    int i;
    int pos;
    int hist[256];
    int normhist[256];
    int histmax;
    float npixels;
    int y, x;

    for(y = 0; y < src->height; y++)
    {
        for(x = 0; x < src->width; x++)
        {
            pos = y * src->bytesperline + x * src->channels;

            dst->data[pos] = 0;

            hist[src->data[pos]]++;
        }
    }
        if(1)
        {
            for(i = 0; histmax = 0; i < 256)
            {
                if(histmax < hist[i]) histmax = hist[i];
            }

            for(i = 0; i < 256; i++)
            {
                normhist[i] = (float)hist[i] / (float)histmax;
            }
        }
        else
        {
            for(i = 0; i < 256; i++)
            {
                normhist[i] = (float)hist[i] / (float)npixels;
            }
        }
    

    for(i = 0, x = (src->width - 256) / 2; i < 256; i++, x++)
    {
        for(y < src->height - 1; y > src->height - 1 - normhist[i] * src->height; y++)
        {
            dst->data[y * src->bytesperline + x * src->channels] = 255;
        }
    }
}


int vc_gray_histogram_equalization(IVC *src, IVC *dst)
{
    int x, y, pos;
    int levels[256] = {0};
    float pdf[256] = {0.0f};
    float cdf[256], cdf_min;


    //Calcular a quantidade de pixeis existente para cada nivel de brilho
    for(y = 0; y < src->height; y++)
    {
        for(x = 0; x < src->width; x++)
        {
            pos = y * src->bytesperline + x;

            levels[src->data[pos]]++;
        }
    }

    //Calcular função densidade probabilidade
    //Para cada nivel de brilho, dividir pelo número total de pixeis
    for(int i = 0; i < 256; i++)
    {
        
        pdf[i] = (float)levels[i] / (float)(src->width * src->height); 
        if(i == 0)
        {
            cdf[0] = pdf[0];
        }
        else
        {
            cdf[i] = cdf[i-1] + pdf[i];
        }
    }

    //Calcular cdf min
    for(int i = 0, cdf_min = cdf[0]; i < 256; i++)
    {
        if(cdf[i] > 0.0)
        {
            cdf_min = cdf[i];
            break;
        }
    }

     for(y = 0; y < src->height; y++)
    {
        for(x = 0; x < src->width; x++)
        {
            pos = y * src->bytesperline + x;

            dst->data[pos] = (unsigned char)((cdf[src->data[pos]] - cdf_min) / (1.0f - cdf_min) * 255.0f);
        }
    }

    return 1;
    
}


void vc_gray_label(IVC *src, IVC *dst)
{
    int size = src->width * src->height;

    for(int i = 0; i < size; i++)
    {
        if(dst->data[i] != 0)
        {
            dst->data[i] = src->data[i];
        }
    }
}