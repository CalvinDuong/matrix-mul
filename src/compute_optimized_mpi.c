#include <omp.h>
#include <x86intrin.h>

#include "compute.h"

int helper(uint32_t bcol, int32_t* pa, int32_t* pb) {
    int y;
    int32_t sum=0;
    
    register __m256i mul = _mm256_setzero_si256();
    register __m256i a;
    register __m256i b;
    register __m256i m;

  
    for(y=0;y<bcol/32*32;y+=32) {
        a = _mm256_loadu_si256((__m256i *) (pa+y));
        b = _mm256_loadu_si256((__m256i *) (pb +y));
        m = _mm256_mullo_epi32(a,b);
        mul = _mm256_add_epi32(m,mul);

        a = _mm256_loadu_si256((__m256i *) (pa+y+8));
        b = _mm256_loadu_si256((__m256i *) (pb+y+8));
        m = _mm256_mullo_epi32(a,b);
        mul = _mm256_add_epi32(m,mul);

        a = _mm256_loadu_si256((__m256i *) (pa+y+16));
        b = _mm256_loadu_si256((__m256i *) (pb +y+16));
        m = _mm256_mullo_epi32(a,b);
        mul = _mm256_add_epi32(m,mul);

        a = _mm256_loadu_si256((__m256i *) (pa+y+24));
        b = _mm256_loadu_si256((__m256i *) (pb+y+24));
        m = _mm256_mullo_epi32(a,b);
        mul = _mm256_add_epi32(m,mul);
        
    }
            
    for(;y<bcol/8*8;y+=8) {
        a = _mm256_loadu_si256((__m256i *) (pa+y));
        b = _mm256_loadu_si256((__m256i *) (pb+ y));
        m = _mm256_mullo_epi32(a,b);
        mul = _mm256_add_epi32(mul,m);
    
    }
            
    int tmp_arr[8];
    _mm256_storeu_si256((__m256i *) tmp_arr,mul);
    
            
    
    for(;y<bcol;y++) {
        
        tmp_arr[0] +=   pa[y] * pb[y];
                
        
    }

    sum +=  tmp_arr[0] + tmp_arr[1] + tmp_arr[2] + tmp_arr[3] + tmp_arr[4] + tmp_arr[5] + tmp_arr[6] + tmp_arr[7];

    

    return sum;
}

            
                

// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
     register int32_t *am = (int32_t*) a_matrix->data;    
     register uint32_t acols =  a_matrix -> cols;
     register uint32_t arows =  a_matrix -> rows;
     register uint32_t bcols =  b_matrix -> cols;
     register uint32_t brows =  b_matrix -> rows;

 
     register int32_t *fbd = (int32_t*) malloc((brows * bcols) * sizeof(int32_t));
    #pragma omp parallel for 
    for(register uint32_t i  = 0; i < (brows * bcols); i++){
        fbd[i] = b_matrix-> data [brows * bcols - i - 1 ];    
   }

    *output_matrix = (matrix_t*)malloc(sizeof(matrix_t));
     uint32_t outrows = arows - brows + 1;
     uint32_t outcols = acols - bcols + 1; 
    (*output_matrix) -> cols  =  outcols;
    (*output_matrix) -> rows =  outrows;

    (*output_matrix) -> data = (int32_t*) malloc(outrows * (outcols)* sizeof(int32_t));

    #pragma omp parallel for collapse(2)
    for(register uint32_t r  = 0; r < outrows ; r++){
        for (register uint32_t c=0; c<outcols;c++) {
            int32_t sum = 0;
            //register int counter = 0;
            for (register uint32_t x = 0 ; x < brows; x++){ //top left of a subpart
                sum+=helper(bcols, am+((r+x)*acols +c), fbd +( bcols*x));
            }

            //printf("%i", sum);
            (*output_matrix) ->data[outcols*r+c] = sum;
            }
        
    }

  return 0;
}

// Executes a task
int execute_task(task_t *task) {
  matrix_t *a_matrix, *b_matrix, *output_matrix;

  char *a_matrix_path = get_a_matrix_path(task);
  if (read_matrix(a_matrix_path, &a_matrix)) {
    printf("Error reading matrix from %s\n", a_matrix_path);
    return -1;
  }
  free(a_matrix_path);

  char *b_matrix_path = get_b_matrix_path(task);
  if (read_matrix(b_matrix_path, &b_matrix)) {
    printf("Error reading matrix from %s\n", b_matrix_path);
    return -1;
  }
  free(b_matrix_path);

  if (convolve(a_matrix, b_matrix, &output_matrix)) {
    printf("convolve returned a non-zero integer\n");
    return -1;
  }

  char *output_matrix_path = get_output_matrix_path(task);
  if (write_matrix(output_matrix_path, output_matrix)) {
    printf("Error writing matrix to %s\n", output_matrix_path);
    return -1;
  }
  free(output_matrix_path);

  free(a_matrix->data);
  free(b_matrix->data);
  free(output_matrix->data);
  free(a_matrix);
  free(b_matrix);
  free(output_matrix);
  return 0;
}
