#include <stdio.h>

#include "flagrtb.h"

int main(int argc, char** argv)
{

  FLAGRTBInfo flagrtb_info;

  flagrtbInfo(&flagrtb_info);

  printf("flagRTB library version: %s\n", flagrtbVersionString());
  printf("Number of polarizations: %u\n", flagrtb_info.npol);
  printf("Number of stations: %u\n", flagrtb_info.nstation);
  printf("Number of baselines: %u\n", flagrtb_info.nbaseline);
  printf("Number of frequencies: %u\n", flagrtb_info.nfrequency);

  printf("Number of time samples per GPU integration: %u\n",
      flagrtb_info.ntime);

  printf("Number of time samples per transfer to GPU: %u\n",
      flagrtb_info.ntimepipe);

  printf("Type of ComplexInput components: ");
  switch(flagrtb_info.input_type) {
    case FLAGRTB_INT8:    printf("8 bit integers\n"); break;
    case FLAGRTB_FLOAT32: printf("32 bit floats\n"); break;
    case FLAGRTB_INT32:   printf("32 bit integers\n"); break;
    default: printf("<unknown type code: %d>\n", flagrtb_info.input_type);
  }

  printf("Number of ComplexInput elements in GPU input vector: %llu\n",
      flagrtb_info.vecLength);

  printf("Number of ComplexInput elements per transfer to GPU: %llu\n",
      flagrtb_info.vecLengthPipe);

  printf("Number of Complex elements in GPU output vector: %llu\n",
      flagrtb_info.matLength);

  printf("Number of Complex elements in reordered output vector: %llu\n",
      flagrtb_info.triLength);

  printf("Output matrix order: ");
  switch(flagrtb_info.matrix_order) {
    case TRIANGULAR_ORDER:               printf("triangular\n");
                                         break;
    case REAL_IMAG_TRIANGULAR_ORDER:     printf("real imaginary triangular\n");
                                         break;
    case REGISTER_TILE_TRIANGULAR_ORDER: printf("register tile triangular\n");
                                         break;
    default: printf("<unknown order code: %d>\n", flagrtb_info.matrix_order);
  }

  printf("Shared atomic transfer size: %lu\n", flagrtb_info.shared_atomic_size);

  printf("Complex block size: %lu\n", flagrtb_info.complex_block_size);

  return 0;
}
