#include <cstring>
#include <iostream>
#include <iterator>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>
#include <vector>

using namespace std;

int main(int argc, char *argv[]){   
	if(argc != 3){
		cout << "Invalid number of arguments." << endl;
		return -1;
	}
	pid_t pipe1[2];
	pid_t pipe2[2];
	if(pipe(pipe1) != 0){
		throw runtime_error(string("failed to create pipe 1: ") + strerror(errno));
	} 
	if(pipe(pipe2) != 0){
		throw runtime_error(string("failed to create pipe 2: ") + strerror(errno));
	} 
	/*int flags_out = fcntl(pipetka_out[0], F_GETFL);
	if (flags_out == -1){
		throw runtime_error(string("Unable to `fcntl(F_GETFL)`:")+ strerror(errno));
	}
	flags_out |= O_NONBLOCK;
	if (fcntl(pipetka_out[0], F_SETFL, flags_out) == -1){
		throw std::runtime_error(string("Unable to `fcntl(F_SETFL)`: ")+strerror(errno));
	}*/


	pid_t child_pid = fork();

	if(child_pid == 0){
		close(pipe1[0]);
		close(pipe2[0]);
		dup2(pipe1[1], STDOUT_FILENO);
		dup2(pipe2[1], STDOUT_FILENO);
		close(pipe[1]);
		close(STDOUT_FILENO);
		execvp(argv[0],argv[0]);
	}       
	cout << pipe1[1] ^ pipe2[1] << endl;

    return 0;    
}
