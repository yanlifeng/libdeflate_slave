extern "C"
{
#include <slave.h>
#include <crts.h>
}

#include "libdeflate.h"

#define BLOCK_SIZE (4 << 20) 
struct Para {
    char* in_buffer;
    char* out_buffer;
    size_t *out_size;
    size_t in_size;
    int level;
};

extern "C" void compressfunc(Para paras[64]) {
    Para *para = &(paras[_PEN]);
    char* in_buffer = para->in_buffer;
    char* out_buffer = para->out_buffer; 
    size_t in_size = para->in_size;
    if(in_size == 0) return;
    
    libdeflate_compressor* compressor = libdeflate_alloc_compressor(para->level);
    size_t bound = libdeflate_gzip_compress_bound(compressor, in_size);
    size_t out_size = libdeflate_gzip_compress(compressor, in_buffer, in_size, out_buffer, bound);
    *(para->out_size) = out_size;
    libdeflate_free_compressor(compressor);

}

extern "C" void decompressfunc(Para paras[64]) {
    Para *para = &(paras[_PEN]);
    char* in_buffer = para->in_buffer;
    char* out_buffer = para->out_buffer; 
    size_t in_size = para->in_size;
    size_t out_size = -1;
    if(in_size == 0) return;
    
    libdeflate_decompressor* decompressor = libdeflate_alloc_decompressor();
    libdeflate_result result = libdeflate_gzip_decompress(decompressor, in_buffer, in_size, out_buffer, BLOCK_SIZE, &out_size);
    if (result != LIBDEFLATE_SUCCESS) {
        fprintf(stderr, "Decompression failed\n");
    }
    *(para->out_size) = out_size;
    libdeflate_free_decompressor(decompressor);

}
