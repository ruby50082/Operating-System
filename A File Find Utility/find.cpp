/*
Student No.: 0516212
Student Name: 吳子涵
Email: ruby50082@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page.
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <iomanip>

using namespace std;

int check[4]={0};
unsigned int inode;
int mini, maxi;
char* name=NULL;

void Find(const char*);

int main(int argc, char** argv){
	string path=argv[1];
	for(int i=2; i<argc; i+=2){
		if(strcmp(argv[i],"-inode")==0){
			inode = atoi(argv[i+1]);
			check[0]=1;
		}
		if(strcmp(argv[i],"-name")==0){
			name = argv[i+1];
			check[1]=1;
		}
		if(strcmp(argv[i],"-size_min")==0){
			mini = atoi(argv[i+1]);
			check[2]=1;
		}
		if(strcmp(argv[i],"-size_max")==0){
			maxi = atoi(argv[i+1]);
			check[3]=1;
		}
	}
	Find(argv[1]);
}

void Find(const char* Path){
	DIR *dir = opendir(Path);
	struct dirent *drt;
	struct stat st;

	string path=Path;
	if(path[path.length()-1] != '/')	
		path.append("/");

	if(dir != NULL){
		while((drt = readdir(dir)) != NULL){
			int found=1;
			if(stat((path + (drt->d_name)).c_str(), &st) == -1)
				continue;
			if(strcmp(drt->d_name, ".")==0 || strcmp(drt->d_name, "..")==0)
				continue;
			if(check[0]==1 && inode != st.st_ino)				found = 0;
			if(check[1]==1 && strcmp(drt->d_name, name))		found = 0;
			if(check[2]==1 && (float)st.st_size/1048576 < mini)	found = 0;
			if(check[3]==1 && (float)st.st_size/1048576 > maxi)	found = 0;
			if(S_ISDIR(st.st_mode) || S_ISREG(st.st_mode)){
				if(found)	cout<<path<<drt->d_name<<" "<<st.st_ino<<" "<<st.st_nlink<<" "<<fixed<<setprecision(6)<<(float)st.st_size/1048576<<"MB "<<ctime(&st.st_ctime);
				if(S_ISDIR(st.st_mode))
					Find((path + (drt->d_name)).c_str());
			}
		}
	}
	closedir(dir);
}