#include <mpi.h>

#include "coordinator.h"

#define READY 0
#define NEW_TASK 1
#define TERMINATE -1

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Error: not enough arguments\n");
    printf("Usage: %s [path_to_task_list]\n", argv[0]);
    return -1;
  }
  int numTasks;
  task_t **tasks;
  if (read_tasks(argv[1], &numTasks, &tasks)) {
          return -1;
  }


    MPI_Init(&argc,&argv);
    int procID, totalProcs;
    // TODO: get the total number of processes and store in `totalProcs`
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);

    // TODO: get the ID of the current program, and store in `procID`
    MPI_Comm_rank(MPI_COMM_WORLD, &procID);
    

    // TODO: check if the current process is the manager
    if (procID==0) {
        // Manager node
        int nextTask = 0;
        MPI_Status status;
        int32_t message;

        // TODO: loop until we've completed `numTasks`
        
        while (numTasks > nextTask) {
            // TODO: receive a message from any source (so we know that this node is done with its task)
            MPI_Recv(&message,1,MPI_UINT32_T,MPI_ANY_SOURCE, 0,MPI_COMM_WORLD, &status);

            // TODO: get the source process using the `status` struct
            int sourceProc = status.MPI_SOURCE;
            // TODO: send `nextTask` as the message to the process we just received a message from
            message = nextTask;
            MPI_Send(&message,1,MPI_UINT32_T,sourceProc, 0, MPI_COMM_WORLD);


            // TODO: increment `nextTask` by 1
            nextTask +=1;
        }

        // Wait for all processes to finish
        // TODO: loop through all processes
        // Hint: we have `totalProcs - 1` total, since there's a manager node
        for (int p=0;p<totalProcs-1;p++) {
            // TODO: receive a message from any source (so we know that this node is done with its task)
            MPI_Recv(&message,1,MPI_UINT32_T,MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

            // TODO: get the source process using the `status` struct
            
            int sourceProc = status.MPI_SOURCE;

            // TODO: send `TERMINATE` as the message to the process we just received a message from
            message = TERMINATE;
            MPI_Send(&message, 1, MPI_UINT32_T, sourceProc, 0, MPI_COMM_WORLD);

        }
    } else {
        // Worker node
        int32_t message;

        while (true) {
            // TODO: let the manager node know that this worker is ready
            message = READY;
            // Hint: use MPI_Send to send a message
            MPI_Send(&message, 1, MPI_UINT32_T, 0, 0, MPI_COMM_WORLD);
            // TODO: receive 1 message from the manager and store it in `message`
            // Hint: use MPI_Recv to receive a message
            MPI_Recv(&message, 1, MPI_UINT32_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // TODO: if the `message` is TERMINATE, break out of the loop to terminate
            if (message == TERMINATE) {
                break; 
            }
            
            if (execute_task(tasks[message])) {
                return -1;
            }
            free(tasks[message]->path);
        }
    }
    free(tasks);
    

    // TODO: call MPI_Finalize since it is the end of the program
    MPI_Finalize();
}
