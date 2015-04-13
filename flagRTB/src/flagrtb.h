#include <stddef.h>

#ifndef FLAGRTB_H
#define FLAGRTB_H

#ifdef __cplusplus
extern "C" {
#endif

// If FIXED_POINT is defined, the library was compiled to use 8-bit fixed
// point (i.e. integers), otherwise it was compiled to use 32-bit floating
// point (i.e. floats).
#define FIXED_POINT

// set the data type accordingly
#ifndef FIXED_POINT
typedef float ReImInput;
#else
typedef char ReImInput;
#endif // FIXED_POINT

typedef struct ComplexInputStruct {
  ReImInput real;
  ReImInput imag;
} ComplexInput;

typedef struct ComplexStruct {
  float real;
  float imag;
} Complex;

// Used to indicate the size and type of input data
#define FLAGRTB_INT8    (0)
#define FLAGRTB_FLOAT32 (1)
#define FLAGRTB_INT32   (2)

// Used to indicate matrix ordering
#define TRIANGULAR_ORDER 1000
#define REAL_IMAG_TRIANGULAR_ORDER 2000
#define REGISTER_TILE_TRIANGULAR_ORDER 3000

// Flags for flagrtbInit
#define FLAGRTB_DEVICE_MASK          ((1<<16)-1)
#define FLAGRTB_DONT_REGISTER_ARRAY  (1<<16)
#define FLAGRTB_DONT_REGISTER_MATRIX (1<<17)
#define FLAGRTB_DONT_REGISTER        (FLAGRTB_DONT_REGISTER_ARRAY | \
                                   FLAGRTB_DONT_REGISTER_MATRIX)

// FLAGRTBInfo is used to convey the compile-time X engine sizing
// parameters of the FLAGRTB library.  It should be allocated by the caller and
// passed (via a pointer) to xInfo() which will fill in the fields.  Note that
// the input values of these fields are currently ignored completely by xInfo()
// (i.e. they are informational only).
typedef struct FLAGRTBInfoStruct {
  // Number of polarizations (NB: will be rolled into a new "ninputs" field)
  unsigned int npol;
  // Number of stations (NB: will be rolled into a new "ninputs" field)
  unsigned int nstation;
  // Number of baselines (derived from nstation)
  unsigned int nbaseline;
  // Number of frequencies
  unsigned int nfrequency;
  // Number of per-channel time samples per integration
  unsigned int ntime;
  // Number of per-channel time samples per transfer to GPU
  unsigned int ntimepipe;
  // Type of input.  One of FLAGRTB_INT8, FLAGRTB_FLOAT32, FLAGRTB_INT32.
  unsigned int input_type;
  // Number of ComplexInput elements in input vector
  long long unsigned int vecLength;
  // Number of ComplexInput elements per transfer to GPU
  long long unsigned int vecLengthPipe;
  // Number of Complex elements in output vector
  long long unsigned int matLength;
  // Number of Complex elements in "triangular order" output vector
  long long unsigned int triLength;
  // Output matrix order
  unsigned int matrix_order;
  // Size of each shared memory transfer
  size_t shared_atomic_size;
  // Number of complex values per real/imag block
  size_t complex_block_size;
} FLAGRTBInfo;

typedef struct FLAGRTBContextStruct {
  // memory pointers on host
  ComplexInput *array_h;
  Complex *matrix_h;

  // Size of memory buffers on host
  size_t array_len;  // in units of sizeof(ComplexInput)
  size_t matrix_len; // in units of sizeof(Complex)

  // Offsets into memory buffers on host.  When calling flagrtbSetHostInputBuffer
  // or flagrtbSetHostOutputBuffer (or functions that call them such as flagrtbInit),
  // these fields are initialized to 0.  When using oversized externally (i.e.
  // caller) allocated host buffers, these fields should be set appropriately
  // prior to calling flagrtbCudaXengine.
  size_t input_offset;
  size_t output_offset;

  // For internal use only
  void *internal;
} FLAGRTBContext;

// Return values from flagrtbCudaXengine()
#define FLAGRTB_OK                          (0)
#define FLAGRTB_OUT_OF_MEMORY               (1)
#define FLAGRTB_CUDA_ERROR                  (2)
#define FLAGRTB_INSUFFICIENT_TEXTURE_MEMORY (3)
#define FLAGRTB_NOT_INITIALIZED             (4)
#define FLAGRTB_HOST_BUFFER_NOT_SET         (5)

// Values for flagrtbCudaXengine's syncOp parameter
#define SYNCOP_NONE           0
#define SYNCOP_DUMP           1
#define SYNCOP_SYNC_TRANSFER  2
#define SYNCOP_SYNC_COMPUTE   3

// Functions in cuda_xengine.cu

// Get pointer to library version string.
//
// The library version string should not be modified or freed!
const char * flagrtbVersionString();

// Get compile-time sizing parameters.
//
// The FLAGRTBInfo structure pointed to by pcxs is populated with
// compile-time sizing parameters.
void flagrtbInfo(FLAGRTBInfo *pcxs);

// Initialize the FLAGRTB.
//
// In addition to allocating device memory and initializing private internal
// context, this routine calls flagrtbSetHostInputBuffer() and
// flagrtbSetHostOutputBuffer().  Be sure to set the context's array_h and
// matrix_h fields and the corresponding length fields) accordingly.
//
// If context->array_h is zero, an array of ComplexInput elements is allocated
// (of the appropriate size) via CudaMallocHost, otherwise the memory region
// pointed to by context->array_h of length context->array_len is registered
// with CUDA via the CudaHostRegister function.
//
// If context->matrix_h is zero, an array of Complex elements is allocated (of
// the appropriate size) via CudaMallocHost, otherwise the memory region
// pointed to by context->matrix_h of length context->matrix_len if registered
// with CUDA via the CudaHostRegister function.
//
// The index of the GPU device should be put into device_flags. In addition,
// the following optional flags can be or'd into this value:
//   FLAGRTB_DONT_REGISTER_ARRAY   Disables registering (pinning) of host array
//   FLAGRTB_DONT_REGISTER_MATRIX  Disables registering (pinning) of host matrix
//   FLAGRTB_DONT_REGISTER         Disables registering (pinning) of all host mem
// E.g., flagrtbInit(&ctx, device_idx | FLAGRTB_DONT_REGISTER_ARRAY);
//
// Note that if registering is disabled, the corresponding flagrtbSetHost*Buffer
// function _must_ be called prior to calling flagrtbCudaXengine.
int flagrtbInit(FLAGRTBContext *context, int device_flags);

// Clear the device integration buffer
//
// Sets the device integration buffer to all zeros, effectively starting a new
// integration.
int flagrtbClearDeviceIntegrationBuffer(FLAGRTBContext *context);

// Specify a new host input buffer.
//
// The previous host input buffer is freed or unregistered (as required) and
// the value in context->array_h is used to specify the new one.  If
// context->array_h is zero, an array of ComplexInput elements is allocated (of
// the appropriate size) via CudaMallocHost and context->array_len is set to
// that value.  Otherwise cudaHostRegister is called to register the region of
// memory pointed to by context->array_h.  The region of memory registered with
// cudaHostRegister starts at context->array_h rounded down to the nearest
// PAGE_SIZE boundary and has a size equal to context->array_len (number of
// complex entries) plus the amount, if any, of rounding down of
// context->array_h all rounded up to the next multiple of PAGE_SIZE.
int flagrtbSetHostInputBuffer(FLAGRTBContext *context);

// Specify a new host output buffer.
//
// The previous host output buffer is freed or unregistered (as required) and
// the value in context->matrix_h is used to specify the new one.  If
// context->matrix_h is zero, an array of Complex elements is allocated (of the
// appropriate size) via CudaMallocHost and context->matrix_len is set to that
// value.  Otherwise cudaHostRegister is called to register the region of
// memory pointed to by context->matrix_h.  The region of memory registered
// with cudaHostRegister starts at context->matrix_h rounded down to the
// nearest PAGE_SIZE boundary and has a size equal to context->matrix_len
// (number of complex entries) plus the amount, if any, of rounding down of
// context->matrix_h all rounded up to the next multiple of PAGE_SIZE.
int flagrtbSetHostOutputBuffer(FLAGRTBContext *context);

void flagrtbFree(FLAGRTBContext *context);

// Perform correlation.  Correlates the input data at (context->array_h +
// context->input_offset).  The syncOp parameter specifies what will be done
// after sending all the asynchronous tasks to CUDA.  The possible values and
// their meanings are:
//
// SYNCOP_NONE - No further action is taken.
// SYNCOP_DUMP - Waits for all transfers and computations to
//               complete, then dumps to output buffer at
//               "context->matrix_h + context->output_offset".
// SYNCOP_SYNC_TRANSFER - Waits for all transfers to complete,
//                        but not necessrily all computations.
// SYNCOP_SYNC_COMPUTE  - Waits for all computations (and transfers) to
//                        complete, but does not dump.
int flagrtbCudaXengine(FLAGRTBContext *context, int syncOp);

// Functions in cpu_util.cc

void flagrtbRandomComplex(ComplexInput* random_num, long long unsigned int length);

void flagrtbReorderMatrix(Complex *matrix);

void flagrtbCheckResult(Complex *gpu, Complex *cpu, int verbose, ComplexInput *array_h);

void flagrtbExtractMatrix(Complex *matrix, Complex *packed);

// Functions in omp_util.cc

void flagrtbOmpXengine(Complex *matrix_h, ComplexInput *array_h);

#ifdef __cplusplus
}
#endif

#endif // FLAGRTB_H
