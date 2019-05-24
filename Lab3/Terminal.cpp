#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main() {
  struct sigaction sa {};
  sa.sa_handler = [](int) {};
  sa.sa_flags = SA_RESTART;
  sigaction(SIGINT, &sa, 0);
  while (1) {
    string command;
    char *cwd = getcwd(0, 0);
    cout << cwd << "> " << flush;
    free(cwd);
    getline(cin, command);
    istringstream ss(command);
    vector<string> vec(istream_iterator<string>(ss),
                       istream_iterator<string>{});
    if (vec.size() == 0) {
      continue;
    }
    vector<char *> ptr_vec;
    for (auto &str : vec) {
      ptr_vec.push_back((char *)str.data());
    }
    ptr_vec.push_back(0);

    if (vec[0] == "cd") {
      if (chdir(ptr_vec[1]) == -1) {
        cout << strerror(errno) << endl;
      }
      continue;
    }
    if (vec[0] == "exit") {
      break;
    }
    bool is_ampersand = false;
    pid_t pipetka_in[2];  // Родительский пишет, дочерний читает
    if (vec.back() == "&") {
      is_ampersand = true;
      if (pipe(pipetka_in) != 0) {
        throw runtime_error(string("failed to create pipe 2: ") +
                            strerror(errno));
      }
      vec.pop_back();
      ptr_vec.erase(ptr_vec.end() - 2);
    }
    pid_t child_pid = fork();
    if (child_pid == 0) {
      if (is_ampersand == true) {
        if (close(pipetka_in[1]) || close(STDIN_FILENO)) {
          throw runtime_error(
              string("failed to close PIPE or file_descriptor: ") +
              strerror(errno));
        }
        if (dup2(pipetka_in[0], STDIN_FILENO)) {
          throw runtime_error(string("failed to duplicate file_descriptor: ") +
                              strerror(errno));
        }
        close(pipetka_in[0]);
      }
      if (is_ampersand == true) {
        struct sigaction sa {};
        sa.sa_handler = SIG_IGN;
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, 0);
      }
      execvp(ptr_vec[0], ptr_vec.data());
    } else if (is_ampersand == true) {
      cout << child_pid << endl;
    }

    if (is_ampersand == false) {
      siginfo_t siginfo{};
      if (waitid(P_PID, child_pid, &siginfo, WEXITED) < 0) {
        throw runtime_error(string("Unable to waitid(): ") + strerror(errno));
      }
    }
  }
  return 0;
}
