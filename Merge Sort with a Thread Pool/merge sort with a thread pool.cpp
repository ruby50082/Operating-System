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
#include <sstream>

using namespace std;

int *arr=NULL, *arr2=NULL;
sem_t sem_write[15], sem_read[7], sem_merge[15], sem_job;
int read_time[7]={0};
int bound[15][2];
const int infinity=2147483647;
queue<int> job_queue;


void* func(void* p);
void bubble(int *arr, int beg, int end);
void merge(int *arr, int beg, int mid, int end);

int main(){
	
	ifstream fin;
	ofstream fout;
	fin.open("input.txt",ifstream::in);
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
		index++;
	}
	fin.close();

	for(int thread_pool=1; thread_pool<=8; thread_pool++){

		//reset
		for(int i=0; i<n; i++)	arr2[i]=arr[i];

		for(int i=0; i<15; i++){
			sem_init(&sem_write[i],0,0);
			sem_init(&sem_merge[i],0,0);
		}
		for(int i=0; i<7; i++)	sem_init(&sem_read[i],0,1);
		sem_init(&sem_job, 0, 1);

		job_queue = queue<int>();
		for(int i=7; i<15; i++)	job_queue.push(i);
		for(int i=6; i>=0; i--)	job_queue.push(i);
		//sort
		bound[0][0]=0;
		bound[0][1]=n-1;
		for(int i=0; i<7; i++){
			bound[2*i+1][0]=bound[i][0];
			bound[2*i+1][1]=bound[i][0]+ceil((bound[i][1]-bound[i][0]+1)/2)-1;
			bound[2*i+2][0]=bound[2*i+1][1]+1;
			bound[2*i+2][1]=bound[i][1];
		}
		
		struct timeval start, end;
		pthread_t thr[15];
		gettimeofday(&start, 0);
		for(int i=0; i<thread_pool; i++){
			pthread_create(&thr[i], NULL, func, (void*)(intptr_t)i);
		}
		sem_wait(&sem_merge[0]);
		gettimeofday(&end, 0);

		int sec, usec;
		sec = end.tv_sec - start.tv_sec;
		usec = end.tv_usec - start.tv_usec;
		cout<<"Elapsed time with "<<thread_pool<<" thread(s): "<<double(sec+double(usec/1000000.0))<<" sec"<<endl;
		
		string filename;
		
		ostringstream ss;
     	ss << thread_pool;
		filename="output_"+ss.str()+".txt";
		
		fout.open(filename.c_str(), ios::out);
		if(!fout){
			cout<<"Can not open the output file\n";
			return 1;
		}
		for(int i=0; i<n; i++){
			fout << arr2[i] << ' ';
		}
		fout.close();
	}
	

	delete [] arr;
	delete [] arr2;
	return 0;

}


void job(int job_id)
{


	if(job_id >= 0 && job_id < 7){
		int child_job_id_1=2*job_id+1;
		int child_job_id_2=2*job_id+2;

		sem_wait(&sem_merge[child_job_id_1]);
		sem_wait(&sem_merge[child_job_id_2]);
		
		merge(arr2, bound[child_job_id_1][0], bound[child_job_id_1][1], bound[child_job_id_2][1]);
		
		sem_post(&sem_merge[job_id]);

		

	}

	else if(job_id >= 7 && job_id < 15){
		if( bound[job_id][1] > bound[job_id][0] ){
			bubble(arr2, bound[job_id][0], bound[job_id][1]);
		}
		sem_post(&sem_write[job_id]);
		sem_post(&sem_merge[job_id]);
		
	}
	
	return;

}

void* func(void* p)
{
	int job_id;

	while(1){
		sem_wait(&sem_job);
		if( job_queue.empty()){
			job_id=-1;
		}
		else{
			job_id=job_queue.front();
			job_queue.pop();
		}
		sem_post(&sem_job);

		if(job_id == -1){
			break;
		}
		else{
			job(job_id);
		}
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



