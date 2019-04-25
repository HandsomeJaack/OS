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

int main(){
    // Отключение ожидания Enter
   /* struct termios data;
    if (tcgetattr(STDIN_FILENO, &data) < 0){
         throw runtime_error(string("Unable to `tcgetattr()`: ")+ strerror(errno));
    }
    data.c_lflag &= ~ICANON;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &data) < 0) {
        throw runtime_error(string("Unable to `tcsetattr()`: ")+ strerror(errno));
    }*/

    while(1){
        string command;
        cout << "> " << flush;
        getline(cin, command);
        istringstream ss(command);
        vector<string> vec(istream_iterator<string>(ss), istream_iterator<string>{});
        if(vec.size() == 0) {
            continue;
        }
        vector<char*> ptr_vec;
        for(auto &str: vec){
            ptr_vec.push_back(str.data());
        }
        ptr_vec.push_back(0);
        
        /*pid_t pipetka_out[2]; //Дочерний пишет, родительский читает
        if(pipe(pipetka_out) != 0){
            throw runtime_error(string("failed to create pipe 1: ") + strerror(errno));
        } 

        pid_t pipetka_in[2]; // Родительский пишет, дочерний читает
        if(pipe(pipetka_in) != 0){
            throw runtime_error(string("failed to create pipe 2: ") + strerror(errno));
        } 

        int flags_out = fcntl(pipetka_out[0], F_GETFL);
        if (flags_out == -1){
            throw runtime_error(string("Unable to `fcntl(F_GETFL)`:")+ strerror(errno));
        }
        flags_out |= O_NONBLOCK;
        if (fcntl(pipetka_out[0], F_SETFL, flags_out) == -1){
            throw std::runtime_error(string("Unable to `fcntl(F_SETFL)`: ")+strerror(errno));
        }
*/
        pid_t child_pid = fork();

        if(child_pid == 0){
            /*close(pipetka_out[0]);
            close(pipetka_in[1]);
            close(STDOUT_FILENO);
            close(STDIN_FILENO);
            dup2(pipetka_out[1], STDOUT_FILENO);
            dup2(pipetka_in[0], STDIN_FILENO);
            close(pipetka_out[1]);
            close(pipetka_in[0]);*/
            execvp(ptr_vec[0], ptr_vec.data());
        }
        siginfo_t siginfo{};
        if (waitid(P_PID, child_pid, &siginfo, WEXITED) < 0)
            throw runtime_error(string("Unable to waitid(): {}") + strerror(errno));
       /* close(pipetka_out[1]);
        close(pipetka_in[0]);
        vector<char> buf(1024);

        while(1){
            while(1){
                int res = read(pipetka_out[0], buf.data(), buf.size());
                if(res < 0){
                    if (errno == EAGAIN)
                    {
                        usleep(10000); // 10 миллисекунд
                        continue;
                    }
                    throw runtime_error(string("reading pipe error: ") + strerror(errno));
                } if(res == 0){
                    break;
                }
                cout.write(buf.data(), res);
            }
            siginfo_t siginfo{};
            if (waitid(P_PID, child_pid, &siginfo, WEXITED | WNOHANG) < 0){
                throw runtime_error(string("Unable to `waitid()`: {}")+ strerror(errno));
            }
            if (siginfo.si_pid){
                // Дочерний закрылся
                break;
            }
        }
        close(pipetka_out[0]);
        close(pipetka_in[1]);
        */
    }
    return 0;    
}