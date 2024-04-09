#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <fstream>
#include "swlu.h"
#include "mpi.h"


#define BLOCK_SIZE (4 << 20) 

extern "C" {
#include <athread.h>
#include <pthread.h>
    void slave_decompressfunc();
    void slave_compressfunc();
}

inline double GetTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double) tv.tv_sec + (double) tv.tv_usec / 1000000;
}

char in_buffer[64][BLOCK_SIZE];
char out_buffer[64][BLOCK_SIZE];

struct Para {
    char* in_buffer;
    char* out_buffer;
    size_t *out_size;
    size_t in_size;
    int level;
};

int main(int argc, char *argv[]) {
    //volatile int aa = 0;
    //while(aa == 0) {
    //
    //}
    //printf("aa = %d\n", aa);


    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <compress_level> [-d]\n", argv[0]);
        return 1;
    }

    bool decompress = argc > 4 || strcmp(argv[3], "-d") == 0;
    //printf("decompress %d\n", decompress);

    FILE *input_file = fopen(argv[1], "rb");
    if (input_file == NULL) {
        perror("Failed to open input file");
        return 1;
    }

    FILE *output_file = fopen(argv[2], "wb");
    if (output_file == NULL) {
        perror("Failed to open output file");
        fclose(input_file);
        return 1;
    }

    int level = decompress ? 0 : argv[3][0] - '0';
    size_t in_size;
    double tot_time = 0;

    std::ifstream iff;
    std::ofstream off;
    if(decompress) {
        std::string in_name(argv[1]);
        iff.open(in_name + ".swidx");
    } else {
        std::string out_name(argv[2]);
        off.open(out_name + ".swidx");
    }

    while (1) {
        int ok = 1;
        Para para[64];
        size_t to_read = 0;
        size_t out_size[64] = {0};  // Can be compressed or decompressed size
        for (int i = 0; i < 64; i++) {
            if(decompress) {
                iff >> to_read; 
                //printf("to_read %d\n", to_read);
                if(iff.eof()) ok = 0;
            } else {
                to_read = BLOCK_SIZE;
            }
            in_size = fread(in_buffer[i], 1, to_read, input_file);
            if (in_size == 0) ok = 0;
            para[i].in_buffer = in_buffer[i];
            para[i].out_buffer = out_buffer[i];
            para[i].in_size = in_size;
            para[i].out_size = &(out_size[i]);
            para[i].level = level;
        }

        double t0 = GetTime();

        if (decompress) {
            __real_athread_spawn((void *)slave_decompressfunc, para, 1);
        } else {
            __real_athread_spawn((void *)slave_compressfunc, para, 1);
        }
        athread_join();

        tot_time += GetTime() - t0;

        for (int i = 0; i < 64; i++) {
            //printf("out_size %d\n", out_size[i]);
            if(!decompress) {
                off << out_size[i] << std::endl;
            }
            if (out_size[i] <= 0) continue;
            if (fwrite(out_buffer[i], 1, out_size[i], output_file) != out_size[i]) {
                perror("Failed to write processed data");
            }
        }
        if (ok == 0) break;
    }

    printf("Total time %.3f\n", tot_time);
    if(decompress) iff.close();
    else off.close();

    fclose(input_file);
    fclose(output_file);
    MPI_Finalize();

    return 0;
}

