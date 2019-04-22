#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string>
#include<cstring>
#include<sstream>
#include<vector>
#include<fcntl.h>
#include<stdio.h>
#include<csignal>

using namespace std;
int main()
{	
	signal(SIGCHLD,SIG_IGN);
	while(1){
		cout<<">";

		string s;
		stringstream ss;
		getline(cin,s);
		int wait=1;
		if(s[s.size()-1]=='&'){
			wait=0;
			s.erase(s.size()-1);
		}

		ss<<s;
		vector<string> v;
		string file;
		vector<string> cmd;
		int red=0;
		int pip=0;
		while(1){
			string tmp;
			ss>>tmp;
			if(ss.fail())	break;
			v.push_back(tmp);
			if(tmp==">"){
				red=1;
				v.pop_back();
			}
			else if(red==1){
				file=tmp;
				v.pop_back();
			}
			if(tmp=="|"){
				pip=1;
				v.pop_back();
			}
			else if(pip==1){
				cmd.push_back(tmp);
				v.pop_back();
			}

		}
		int fd;
		if(red==1){
			remove(file.c_str());
			fd=open(file.c_str(),O_WRONLY | O_CREAT,0644);
		}

		int fd_p[2];
		pipe(fd_p);

		int num;
		num=v.size();
		char* arg[num+1];
		int cmd_num;
		cmd_num=cmd.size();
		char* arg_p[cmd_num+1];
	
		for(int i=0;i<num+1;i++){
			arg[i]= new char[300];
		}
		for(int i=0;i<cmd_num+1;i++){
			arg_p[i]= new char[300];
		}
		for(int i=0;i<num;i++){
			strcpy(arg[i], v[i].c_str());
		}
		for(int i=0;i<cmd_num;i++){
			strcpy(arg_p[i], cmd[i].c_str());
		}
		arg[num]=NULL;
		arg_p[cmd_num]=NULL;
		pid_t pid;
		pid = fork();
		if (pid < 0) {
			cout<<"Fork Failed"<<endl;
		}
		else if (pid == 0) {
			if(red==1){
				dup2(fd,STDOUT_FILENO);
				execvp(arg[0],arg);
			}
			else if(pip==1){
				close(fd_p[0]);
				dup2(fd_p[1],1);
				close(fd_p[1]);
				execvp(arg[0],arg);
			}
			else{
				execvp(arg[0],arg);
			}
		}
		else {
			if(wait==1){
				int status;
				waitpid(pid,&status,0);
			}
			if(pip==1){
				close(fd_p[1]);
				dup2(fd_p[0],0);
				close(fd_p[0]);
				execvp(arg_p[0],arg_p);
			}
		}
		if(red==1) close(fd);
	}
}
