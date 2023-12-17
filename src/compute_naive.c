#include "compute.h"

// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
    
    matrix_t* flipped_b = (matrix_t*)malloc(sizeof(matrix_t));
    flipped_b -> rows = b_matrix -> rows;
    flipped_b -> cols = b_matrix -> cols; 
    flipped_b-> data = (int*) malloc((flipped_b -> rows * flipped_b ->cols) * sizeof(int));
    for(int i  = 0; i < (flipped_b -> rows * flipped_b -> cols); i++){
        flipped_b->data[i] = b_matrix-> data [b_matrix-> rows * b_matrix -> cols - i - 1 ];    
    }
    
    *output_matrix = (matrix_t*)malloc(sizeof(matrix_t));
    (*output_matrix) -> rows = a_matrix -> rows - flipped_b -> rows + 1;
    (*output_matrix) -> cols = a_matrix -> cols - flipped_b -> cols + 1; 
    (*output_matrix) -> data = (int*) malloc((*output_matrix) -> rows * ((*output_matrix) -> cols) * sizeof(int));

    int jump = a_matrix -> cols - (*output_matrix) -> cols;
    int j = 0;
    int k =0;
    
    for(int i  = 0; i < (*output_matrix) -> rows * ((*output_matrix) -> cols); i++){
        int sum = 0; 
        int counter = 0;
         
        
        if (k == (*output_matrix) -> cols){
            k = 0;
            j = j + jump;
            
        }
                
        //printf("j:");
       // printf("%d", j);
        //printf("\n");
        for (int x = j ; x < j+((b_matrix) -> rows) * a_matrix -> cols; x += a_matrix -> cols){ //top left of a subpart
            for( int y = 0; y < flipped_b -> cols; y ++){ //
                //printf("%d", a_matrix -> data[x+y] );
                sum += a_matrix -> data[x+y] * flipped_b -> data[counter];
                counter += 1; 
                //printf("\n"); 
            }
        }

        //printf("%d", sum);
//printf("\n"); 
        (*output_matrix) ->data[i] = sum;
        
        k +=1;
        j+=1;
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
