#include <iostream>
#include <fstream>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <semaphore.h>
#include <string>
#include <algorithm>
#include <math.h>
#include <cstring>
#include <queue>

using namespace std;

int *arr=NULL, *arr2=NULL;
sem_t sem_write[15], sem_read[7], sem_merge[15];
int read_time[7]={0};
int bound[15][2];
const int infinity=2147483647;

void* func(void* p);
void bubble(int *arr, int beg, int end);
void merge(int *arr, int beg, int mid, int end);

int main(){
	
	string file;
	cout << "Please input the file name: ";
	cin >> file;
	ifstream fin;
	ofstream fout;
	fin.open(file.c_str(),ifstream::in);
	if(!fin){
		cout<<"Can not open the file\n";
		return 1;
	}
	int n,index=0;
	fin>>n;

	arr = new int[n];
	arr2 = new int[n];
	while(!fin.eof()){
		fin>>arr[index];
		arr2[index]=arr[index];
		index++;
	}
	fin.close();

	for(int i=0; i<15; i++){
		sem_init(&sem_write[i],0,0);
		sem_init(&sem_merge[i],0,0);
	}
	for(int i=0; i<7; i++)	sem_init(&sem_read[i],0,1);

	// MT-sorting
	bound[0][0]=0;
	bound[0][1]=n-1;
	struct timeval start, end;
	pthread_t thr[15];
	gettimeofday(&start, 0);
	for(int i=0; i<15; i++){
		pthread_create(&thr[i], NULL, func, (void*)(intptr_t)i);
	}

	sem_wait(&sem_merge[0]);

	gettimeofday(&end, 0);

	int sec, usec;
	sec = end.tv_sec - start.tv_sec;
	usec = end.tv_usec - start.tv_usec;
	cout<<"Elapsed time of MT sorting: "<<double(sec+double(usec/1000000.0))<<" sec"<<endl;
	fout.open("output1.txt", ios::out);
	if(!fout){
		cout<<"Can not open the MT-output file\n";
		return 1;
	}
	for(int i=0; i<n; i++){
		fout << arr[i] << ' ';
	}
	fout.close();

	// ST-sorting
	gettimeofday(&start, 0);
	for(int i=7; i<=14; i++){
		bubble(arr2, bound[i][0], bound[i][1]);
	}
	for(int i=0; i<8; i=i+2){
		merge(arr, bound[7+i][0], bound[7+i][1], bound[8+i][1]);
	}
	for(int i=0; i<4; i=i+2){
		merge(arr, bound[3+i][0], bound[3+i][1], bound[4+i][1]);
	}
	merge(arr, bound[1][0], bound[1][1], bound[2][1]);

	gettimeofday(&end, 0);
	sec = end.tv_sec - start.tv_sec;
	usec = end.tv_usec - start.tv_usec;
	cout<<"Elapsed time of ST sorting: "<<double(sec+double(usec/1000000.0))<<" sec"<<endl;
	fout.open("output2.txt", ios::out);
	if(!fout){
		cout<<"Can not open the ST-output file\n";
		return 1;
	}
	for(int i=0; i<n; i++){
		fout << arr[i] << ' ';
	}
	fout.close();

	delete [] arr;
	delete [] arr2;
	return 0;

}


void* func(void* p)
{
	int tid=*((int*)&p);
	int parent_tid=(tid-1)/2;
	int child_id_1=2*tid+1;
	int child_id_2=2*tid+2;
	if(tid >= 0 && tid < 7){
		if(tid != 0){
			sem_wait(&sem_read[parent_tid]);
			read_time[parent_tid]++;
			if(read_time[parent_tid]==1){
				sem_wait(&sem_write[parent_tid]);
			}
			sem_post(&sem_read[parent_tid]);
		}

		bound[2*tid+1][0]=bound[tid][0];
		bound[2*tid+1][1]=bound[tid][0]+ceil((bound[tid][1]-bound[tid][0]+1)/2)-1;
		bound[2*tid+2][0]=bound[2*tid+1][1]+1;
		bound[2*tid+2][1]=bound[tid][1];
		
		sem_post(&sem_write[tid]);

		sem_wait(&sem_merge[child_id_1]);
		sem_wait(&sem_merge[child_id_2]);
		
		merge(arr, bound[child_id_1][0], bound[child_id_1][1], bound[child_id_2][1]);
		sem_post(&sem_merge[tid]);

	}

	else if(tid >= 7 && tid < 15){
		// wait for parent
		sem_wait(&sem_read[parent_tid]);
		read_time[parent_tid]++;
		if(read_time[parent_tid]==1){
			sem_wait(&sem_write[parent_tid]);
		}
		sem_post(&sem_read[parent_tid]);

		if( bound[tid][1] > bound[tid][0] ){
			bubble(arr, bound[tid][0], bound[tid][1]);
		}
		sem_post(&sem_write[tid]);
		sem_post(&sem_merge[tid]);
	}
	
	pthread_exit((void*)0);


}

void bubble(int *arr, int beg, int end){
	for(int i=beg; i<=end; i++){
		for(int j=end; j>i; j--){
			if(arr[i] > arr[j]){
				swap(arr[i], arr[j]);
			}
		}
	}
}

void merge(int *arr, int beg, int mid, int end){
	queue<int> q1,q2;
	for(int i=0; i < mid-beg+1; i++){
		q1.push(arr[beg+i]);
	}
	for(int i=0; i < end-mid; i++){
		q2.push(arr[mid+1+i]);
	}
	
	q1.push(infinity);
	q2.push(infinity);

	for(int i=beg; i<=end; i++){
		if(q1.front() < q2.front()){
			arr[i]=q1.front();
			q1.pop();
		}
		else{
			arr[i]=q2.front();
			q2.pop();
		}
	}
}



