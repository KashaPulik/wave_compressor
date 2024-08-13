#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct wav_header
{
    char ChunkID[5];
    unsigned int ChunkSize;
    char Format[5];
    char Subchunk1ID[5];
    unsigned int Subchunk1Size;
    unsigned short int AudioFormat;
    unsigned short int NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short int BlockAlign;
    unsigned short int BitsPerSample;
} WAV_header;

int find_data_field(char *filename)
{
    if (filename == NULL)
        return -1;
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        return -1;
    char buffer;
    int count = 0;
    while (fread(&buffer, 1, 1, file) == 1)
    {
        if (buffer == 'd')
        {
            fread(&buffer, 1, 1, file);
            if (buffer == 'a')
            {
                fread(&buffer, 1, 1, file);
                if (buffer == 't')
                {
                    fread(&buffer, 1, 1, file);
                    if (buffer == 'a')
                    {
                        fclose(file);
                        return count;
                    }
                    count++;
                }
                count++;
            }
            count++;
        }
        count++;
    }
    fclose(file);
    return -1;
}

unsigned char *get_header(char *filename, int *count)
{
    if (filename == NULL)
        return NULL;
    *count = find_data_field(filename);
    if (*count == -1)
        return NULL;
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        return NULL;
    unsigned char *header = malloc(*count);
    if (fread(header, 1, *count, file) != *count)
    {
        free(header);
        fclose(file);
        return NULL;
    }
    fclose(file);
    return header;
}

unsigned short int get_two_bytes(unsigned char *array)
{
    unsigned short int number = array[0];
    number |= ((int)array[1]) << 8;
    return number;
}

unsigned int get_four_bytes(unsigned char *array)
{
    unsigned int number = array[0];
    number |= ((int)array[1]) << 8;
    number |= ((int)array[2]) << 16;
    number |= ((int)array[3]) << 24;
    return number;
}

WAV_header fill_header(unsigned char *header)
{
    WAV_header this_header;
    this_header.ChunkID[0] = header[0];
    this_header.ChunkID[1] = header[1];
    this_header.ChunkID[2] = header[2];
    this_header.ChunkID[3] = header[3];
    this_header.ChunkID[4] = '\0';
    this_header.ChunkSize = get_four_bytes(&header[4]);
    this_header.Format[0] = header[8];
    this_header.Format[1] = header[9];
    this_header.Format[2] = header[10];
    this_header.Format[3] = header[11];
    this_header.Format[4] = '\0';
    this_header.Subchunk1ID[0] = header[12];
    this_header.Subchunk1ID[1] = header[13];
    this_header.Subchunk1ID[2] = header[14];
    this_header.Subchunk1ID[3] = header[15];
    this_header.Subchunk1ID[4] = '\0';
    this_header.Subchunk1Size = get_four_bytes(&header[16]);
    this_header.AudioFormat = get_two_bytes(&header[20]);
    this_header.NumChannels = get_two_bytes(&header[22]);
    this_header.SampleRate = get_four_bytes(&header[24]);
    this_header.ByteRate = get_four_bytes(&header[28]);
    this_header.BlockAlign = get_two_bytes(&header[32]);
    this_header.BitsPerSample = get_two_bytes(&header[34]);
    return this_header;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Arguments incorrect!\n");
        printf("Use \"./get_header <filename>\"\n");
        return 1;
    }
    int count;
    unsigned char *header = get_header(argv[1], &count);
    WAV_header this_header = fill_header(header);
    free(header);
    printf("Filename: \"%s\"\n", argv[1]);
    printf("ChunkID: %s\n", this_header.ChunkID);
    printf("ChunkSize: %d\n", this_header.ChunkSize);
    printf("Format: %s\n", this_header.Format);
    printf("Subchunk1ID: %s\n", this_header.Subchunk1ID);
    printf("Subchunk1Size: %d\n", this_header.Subchunk1Size);
    printf("AudioFormat: %d\n", this_header.AudioFormat);
    printf("NumChannels: %d\n", this_header.NumChannels);
    printf("SampleRate: %d\n", this_header.SampleRate);
    printf("ByteRate: %d\n", this_header.ByteRate);
    printf("BlockAlign: %d\n", this_header.BlockAlign);
    printf("BitsPerSample: %d\n", this_header.BitsPerSample);
}