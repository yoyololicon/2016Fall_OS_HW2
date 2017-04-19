#include <iostream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
using namespace std;

uint32_t *initMatrix(int id, int dimension)
{
    uint32_t *A = (uint32_t*)shmat(id, NULL, 0);
    for(uint32_t i = 0 ; i < dimension*dimension ; i++)
        A[i] = i ;
    return A;
}

void process(int &c, int start, int dis, int dms, int rem, uint32_t *A, uint32_t *B, uint32_t *C)
{
    c--;
    pid_t pid = fork();
    if (pid < 0) {
        cout << "Fork Failed" << endl;
        _exit(EXIT_FAILURE);
    } else if (pid == 0) {
        uint32_t sum = 0;
        for(uint32_t i = 0 ; i < dis ; i++)
        {
            for(uint32_t j = 0 ; j < dms ; j++)
            {
                for(uint32_t k = 0 ; k < dms ; k++)
                {
                    sum+=*(A+i*dms+k)* *(B+k*dms+j);
                }
                *(C+i*dms+j) = sum;
                sum = 0;
            }
        }
        _exit(EXIT_SUCCESS);
    } else {
        if(c){
            if(c==1 && rem != 0)
                process(c, start+dis, rem, dms, 0, A, B, C);
            else
                process(c, start+dis, dis, dms, rem, A, B, C);
        }
        wait(NULL);
    }
}

int main() {
    uint32_t sum, *A, *B, *C;
    struct timeval start, end;
    int sec, usec;
    pid_t pid;
    int shmidA, shmidB, shmidC, dms, count, div, rem;

    cout << "Input the matrix dimension: ";
    cin >> dms;
    shmidA = shmget(IPC_PRIVATE, dms*dms* sizeof(uint32_t), IPC_CREAT | 0666);
    shmidB = shmget(IPC_PRIVATE, dms*dms* sizeof(uint32_t), IPC_CREAT | 0666);
    shmidC = shmget(IPC_PRIVATE, dms*dms* sizeof(uint32_t), IPC_CREAT | 0666);

    A = initMatrix(shmidA, dms);
    B = initMatrix(shmidB, dms);
    C = (uint32_t*)shmat(shmidC, NULL, 0);

    for(int i = 0 ; i < 16 ; i++)
    {
        cout << "Multiplying matrices using " << i+1 << " process" << endl;

	count = i + 1;
        div = dms/count;
        rem = dms%count;

        gettimeofday(&start, 0);      
        process(count, 0, div, dms, rem, A, B, C);
	gettimeofday(&end, 0);
        
	sum = 0 ;
        for(int i = 0 ; i < dms*dms ; i++)
            sum+=C[i];
        sec = end.tv_sec - start.tv_sec;
        usec = end.tv_usec - start.tv_usec;
        cout << "Elapsed time: " << sec+(usec/1000000.0) << " sec, Checksum: " << sum << endl;
    }
    shmctl(shmidA, IPC_RMID, 0);
    shmctl(shmidB, IPC_RMID, 0);
    shmctl(shmidC, IPC_RMID, 0);

    return 0;
}
