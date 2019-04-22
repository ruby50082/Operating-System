#include <iostream>
#include <stdlib.h>
#include <unordered_map>
#include <fstream>
#include <list>
#include <sys/time.h>
#include <vector>
using namespace std;

void LRU(int);
void LFU(int);
vector <int> addr;

int main(int argc,char **argv){
	ifstream fin;
	struct timeval start, end;

	printf("LFU policy:\n");
	printf("frame\thit\t\tmiss\t\tpage fault ratio\n");
	gettimeofday(&start, 0);
	for(int i=128; i<=1024; i*=2){
		addr.clear();
		fin.open(argv[1],ifstream::in);
		if(!fin)	printf("error open\n");
		int tmp;
		while(fin>>tmp){
			if(tmp+'0'=='\n')	break;
			addr.push_back(tmp);
			if(fin.eof())	break;
		}
		LFU(i);
		fin.close();
	}
	gettimeofday(&end, 0);
	int sec, usec;
	sec = end.tv_sec - start.tv_sec;
	usec = end.tv_usec - start.tv_usec;
	printf("Total elapsed time: %.4f sec\n\n", double(sec+double(usec/1000000.0)));
	fin.close();

	printf("LRU policy:\n");
	printf("frame\thit\t\tmiss\t\tpage fault ratio\n");
	gettimeofday(&start, 0);
	for(int i=128; i<=1024; i*=2){
		addr.clear();
		fin.open(argv[1],ifstream::in);
		if(!fin)	printf("error open\n");
		int index=0;
		int tmp;
		while(fin>>tmp){
			if(tmp+'0'=='\n')	break;
			addr.push_back(tmp);
			if(fin.eof())	break;
		}
		LRU(i);
		fin.close();
	}
	
	gettimeofday(&end, 0);
	sec = end.tv_sec - start.tv_sec;
	usec = end.tv_usec - start.tv_usec;
	printf("Total elapsed time: %.4f sec\n", double(sec+double(usec/1000000.0)));
}

void LFU(int frame_num){
	unordered_map <int, int> lookup_table;	//addr frequency
	unordered_map <int, list<pair<int, int>>> frame;     	//frequency addr_list
	unordered_map <int ,list<pair<int, int>>::iterator> iter;//addr position
	int hit=0, miss=0;
	int min_freq=1;
	int num=0;
	int current_time = 0;

	for(int i=0; i<addr.size(); i++){
		if(lookup_table.find(addr[i])==lookup_table.end()){	//not exist in lookup-table
			if (num==frame_num){	//full
				if(min_freq ==1 ){
					pair<int,int> victim=frame[min_freq].back();
					frame[min_freq].pop_back();
					lookup_table.erase(victim.first);
					iter.erase(victim.first);
				}
				else{
					pair<int,int> smallest(0,2147483647);
					for(auto now:frame[min_freq]){
						if(now.second < smallest.second)
							smallest = now;
					}
					frame[min_freq].erase(iter[smallest.first]);
					lookup_table.erase(smallest.first);
					iter.erase(smallest.first);
				}
				num--;
			}
			min_freq=1;
			frame[1].push_front(make_pair(addr[i],current_time));
			lookup_table.insert( pair<int, int> (addr[i], 1) );
			iter[addr[i]]=frame[1].begin();
			num++;
			miss++;
		}
		else{						//exist in lookup-table
			int f=lookup_table[addr[i]];
			auto old = iter[addr[i]];
			int t = (*old).second;
			frame[f].erase(iter[addr[i]]);
			if(f==min_freq && frame[f].empty()){
				min_freq++;
			}
			frame[f+1].push_front(make_pair(addr[i],t));
			iter[addr[i]]=frame[f+1].begin();
			lookup_table[addr[i]]++;
			hit++;
		}
		current_time ++;
		
	}
	float ratio = (float)miss / (float)(miss+hit);
	printf("%d\t%d\t\t%d\t\t%-.10f\n", frame_num, hit, miss, ratio);
	
}

void LRU(int frame_num){
    unordered_map<int, list<int>::iterator> lookup_table;
	list<int> frame;
	int hit=0, miss=0;

	for(int i=0; i<addr.size(); i++){
		if(lookup_table.find(addr[i])==lookup_table.end()){
			if(frame.size() == frame_num){
                lookup_table.erase(*frame.begin());
                frame.pop_front();
            }

            list <int>::iterator it=frame.end();
			frame.push_back(addr[i]);
			lookup_table.insert( pair<int, list<int>::iterator> (addr[i], --it) );
	        miss++;
		}
		else{
			list<int>::iterator it = lookup_table[addr[i]];
        	frame.splice(frame.end(), frame, it);
        	lookup_table[addr[i]] = --frame.end();
	        hit++;
		}
	}
	float ratio = (float)miss / (float)(miss+hit);
	printf("%d\t%d\t\t%d\t\t%-.10f\n", frame_num, hit, miss, ratio);

}
