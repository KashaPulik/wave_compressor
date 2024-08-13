#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

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
    char Subchunk2ID[5];
    unsigned int Subchunk2Size;
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
    *count += 8;
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

WAV_header fill_header(unsigned char *header, int size)
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
    this_header.Subchunk2ID[0] = header[size - 8];
    this_header.Subchunk2ID[1] = header[size - 7];
    this_header.Subchunk2ID[2] = header[size - 6];
    this_header.Subchunk2ID[3] = header[size - 5];
    this_header.Subchunk2ID[4] = '\0';
    this_header.Subchunk2Size = get_four_bytes(&header[size - 4]);
    return this_header;
}

void print_meta(WAV_header header, char *filename)
{
    printf("Filename: \"%s\"\n", filename);
    printf("ChunkID: %s\n", header.ChunkID);
    printf("ChunkSize: %d\n", header.ChunkSize);
    printf("Format: %s\n", header.Format);
    printf("Subchunk1ID: %s\n", header.Subchunk1ID);
    printf("Subchunk1Size: %d\n", header.Subchunk1Size);
    printf("AudioFormat: %d\n", header.AudioFormat);
    printf("NumChannels: %d\n", header.NumChannels);
    printf("SampleRate: %d\n", header.SampleRate);
    printf("ByteRate: %d\n", header.ByteRate);
    printf("BlockAlign: %d\n", header.BlockAlign);
    printf("BitsPerSample: %d\n", header.BitsPerSample);
    printf("Subchunk2ID: %s\n", header.Subchunk2ID);
    printf("Subchunk2Size: %d\n", header.Subchunk2Size);
}

int *get_data(char *filename, WAV_header header, int header_size)
{
    if (filename == NULL)
        return NULL;
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        return NULL;
    int BytesPerSample = header.BitsPerSample / 8;
    int sample_count = header.Subchunk2Size / BytesPerSample;
    int *data = malloc(sample_count * sizeof(int));
    fseek(file, header_size, SEEK_SET);
    fread(data, BytesPerSample, sample_count, file);
    fclose(file);
    return data;
}

int get_count_of_chunks_in_channel(WAV_header header)
{
    int count = header.Subchunk2Size / (header.BitsPerSample / 8) / header.NumChannels;
    return count;
}

int *get_left_channel(int *data, int size)
{
    int *left_channel = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++)
        left_channel[i] = data[i * 2];
    return left_channel;
}

int *get_right_channel(int *data, int size)
{
    int *right_channel = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++)
        right_channel[i] = data[i * 2 + 1];
    return right_channel;
}

char *compress_channel(int *channel, int size)
{
    char *compressed = malloc(size - 1);
    double y2, cos;
    for (int i = 0; i < size - 1; i++)
    {
        y2 = (double)(channel[i + 1] - channel[i]);
        cos = y2 / (sqrt(1.0 + y2 * y2));
        cos *= 127.0;
        compressed[i] = (char)round(cos);
    }
    return compressed;
}

void write_file(char *filename, int left_start, int right_start, char *c_left_channel, char *c_right_channel, int channel_size, unsigned char *header, int header_size)
{
    FILE *file = fopen(filename, "wb");
    fwrite(header, 1, header_size, file);
    fwrite(&left_start, 2, 1, file);
    fwrite(&right_start, 2, 1, file);
    for (int i = 0; i < channel_size - 1; i++)
    {
        fwrite(c_left_channel, 1, 1, file);
        fwrite(c_right_channel, 1, 1, file);
    }
    fclose(file);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Arguments incorrect!\n");
        printf("Use \"./compress <input> <output>\"\n");
        return 1;
    }
    int header_size;
    unsigned char *raw_header = get_header(argv[1], &header_size);
    WAV_header header = fill_header(raw_header, header_size);
    int *data = get_data(argv[1], header, header_size);
    int chunks_in_channel = get_count_of_chunks_in_channel(header);
    int *left_channel = get_left_channel(data, chunks_in_channel);
    int *right_channel = get_right_channel(data, chunks_in_channel);
    char *c_left_channel = compress_channel(left_channel, chunks_in_channel);
    char *c_right_channel = compress_channel(right_channel, chunks_in_channel);
    write_file(argv[2], left_channel[0], right_channel[0], c_left_channel, c_right_channel, chunks_in_channel, raw_header, header_size);
}