#include <iostream>
#include <wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

using namespace std;

unsigned int multiply(int a, int b, int c, int d, int n);

int shmid_A, shmid_B, shmid_C, shmid_S;
unsigned int *A, *B, *C, *S;

int main(){
	int n;
	cout<<"Please input the dimension of the matrix:";
	cin>>n;

	shmid_A=shmget(1000, sizeof(unsigned int)*n*n, IPC_CREAT | 0666);
	if(shmid_A == -1){
		perror("shmget error");
		exit(EXIT_FAILURE);
	}
	A = (unsigned int*)shmat(shmid_A,NULL,0);

	shmid_B=shmget(1200, sizeof(unsigned int)*n*n, IPC_CREAT | 0666);
	if(shmid_B == -1){
		perror("shmget error");
		exit(EXIT_FAILURE);;
	}
	B = (unsigned int*)shmat(shmid_B,NULL,0);

	shmid_C=shmget(1400, sizeof(unsigned int)*n*n, IPC_CREAT | 0666);
	if(shmid_B == -1){
		perror("shmget error");
		exit(EXIT_FAILURE);;
	}

	
	for(int i=0; i<n*n; i++){
		A[i]=i;
		B[i]=i;
	}
	
	shmdt(A);
	shmdt(B);

	struct timeval start, end;

	//1
	gettimeofday(&start, NULL);
	A = (unsigned int*)shmat(shmid_A,NULL,0);
	B = (unsigned int*)shmat(shmid_B,NULL,0);
	C = (unsigned int*)shmat(shmid_C,NULL,0);
	
	for(int i=0; i<n*n; i++)	C[i]=0;

	unsigned int sum=0;
	sum=multiply(0,n,0,n,n);
	
	shmdt(A);
	shmdt(B);
	shmdt(C);
	gettimeofday(&end, NULL);
	
	int sec,usec;
	sec = end.tv_sec-start.tv_sec;
	usec = end.tv_usec-start.tv_usec;
	cout << "Multiplying matrices using 1 process\n";
	cout << "Elapsed time:" << double(sec+double(usec/1000000.0)) << "sec, Checksum:" << sum << endl;
	

	//2~16
	pid_t pid;
	for(int process=2; process<=16; process++){

		gettimeofday(&start, NULL);
		C = (unsigned int*)shmat(shmid_C,NULL,0);
		for(int i=0; i<n*n; i++){
			C[i]=0;
		}
		shmid_S=shmget(1600, sizeof(unsigned int)*n*n, IPC_CREAT | 0666);
		S = (unsigned int*)shmat(shmid_S,NULL,0);
		for(int i=0; i<16; i++)		S[i]=0;
		
		for(int child=0; child < process; child++){
			pid=fork();
			if(pid==0){
				A = (unsigned int*)shmat(shmid_A,NULL,0);
				B = (unsigned int*)shmat(shmid_B,NULL,0);
				C = (unsigned int*)shmat(shmid_C,NULL,0);
				S = (unsigned int*)shmat(shmid_S,NULL,0);
				
				int bound;
				if(child+1==process){
					bound=n;
				}
				else{
					bound=(child+1)*(n/process);
				}
				
				S[child]=multiply(child*(n/process),bound,0,n,n);
				shmdt(A);
				shmdt(B);
				shmdt(C);
				shmdt(S);
				exit(0);
			}
			
		}

		for(int child=0; child < process; child++){
			wait(NULL);
		}
		S = (unsigned int*)shmat(shmid_S,NULL,0);
		unsigned int ss=0;
		for(int i=0; i<process; i++){
			ss+=S[i];
		}
		
		shmdt(C);
		gettimeofday(&end, 0);
		
		int sec,usec;
		sec = end.tv_sec-start.tv_sec;
		usec = end.tv_usec-start.tv_usec;
		cout << "Multiplying matrices using "<< process <<" processes\n";
		cout << "Elapsed time:" << double(sec+double(usec/1000000.0)) << "sec, Checksum:" << ss << endl;
		shmdt(S);
	
	}

	shmctl(shmid_A, IPC_RMID, 0);
	shmctl(shmid_B, IPC_RMID, 0);
	shmctl(shmid_C, IPC_RMID, 0);
	shmctl(shmid_S, IPC_RMID, 0);
	return 0;

}


unsigned int multiply(int a, int b, int c, int d, int n)
{	
	for(int i=a; i<b; i++){
		for(int j=c; j<d; j++){
			for(int k=0; k<n; k++){
				C[i*n+j] += A[i*n+k] * B[k*n+j];
				
			}
		}
	}
	unsigned int s=0;
	for(int i=a; i<b; i++){
		for(int j=c; j<d; j++){
			s+=C[i*n+j];
		}
	}
	return s;
}
