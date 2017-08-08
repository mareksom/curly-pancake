#line 2 ".suffix.cpp"

}  // namespace solution

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace utils {

void StrCat_(std::ostream& stream) {}
template <typename Arg1, typename ...Args>
void StrCat_(std::ostream& stream, Arg1&& arg1, Args&& ...args) {
  stream << std::forward<Arg1>(arg1);
  StrCat_(stream, std::forward<Args>(args)...);
}

template <typename ...Args>
std::string StrCat(Args&& ...args) {
  std::stringstream stream;
  StrCat_(stream, std::forward<Args>(args)...);
  return stream.str();
}

}  // namespace utils

namespace colors {

constexpr char Reset[]  = "\033[0m";
constexpr char Red[]    = "\033[31m";
constexpr char Green[]  = "\033[32m";
constexpr char Yellow[] = "\033[33m";
constexpr char Blue[]   = "\033[34m";

}  // namespace colors

namespace status_bar {

bool is_enabled;
int width, height;
std::vector<std::string> symbols;

void Goto(int row, int col) {
  if (row <= 0) row += height;
  std::cerr << "\033[" << row << ";" << col << "H";
}

void PrintBar() {
  std::cerr << "[";
  for (const std::string& symbol : symbols) {
    std::cerr << symbol;
  }
  std::cerr << "]" << std::endl;
}

void Initialize(bool is_enabled_arg, int size) {
  winsize w;
  if (!is_enabled_arg or isatty(1 /* stdin */) or !isatty(2 /* stderr */) or
      ioctl(STDERR_FILENO, TIOCGWINSZ, &w) == -1) {
    is_enabled = false;
    return;
  }
  is_enabled = true;
  height = w.ws_row;
  width = w.ws_col;
  symbols.resize(size, " ");
  PrintBar();
}

void UpdateSymbol(int i, const std::string& symbol) {
  if (!is_enabled) {
    return;
  }
  assert(0 <= i and i < static_cast<int>(symbols.size()));
  symbols[i] = symbol;
  Goto(-1, 2 + i);
  std::cerr << symbol;
  Goto(0, 1);
}

void PrintMessage(const std::string& message) {
  if (!is_enabled) {
    std::cerr << message << std::endl;
    return;
  }
  Goto(-1, 1);
  for (int i = 0; i < static_cast<int>(symbols.size()) + 2; i++) {
    std::cerr << " ";
  }
  Goto(-1, 1);
  std::cerr << message << std::endl;
  PrintBar();
}

}  // namespace status_bar

template <typename ...Args>
void Error(Args&& ...args);

#define Perror(args...)                                        \
    Error("Line ", __LINE__, ": ",                             \
          args, " | error = [", errno, "] ", strerror(errno))

namespace systm {

constexpr int kBufferSize = 4096;
char buffer[kBufferSize];

class Pipe {
 public:
  Pipe() : pipe_{-1, -1} {}

  Pipe(const Pipe&) = delete;
  Pipe& operator=(const Pipe&) = delete;

  Pipe(Pipe&& pipe) : pipe_{pipe.pipe_[0], pipe.pipe_[1]} {
    pipe.pipe_[0] = pipe.pipe_[1] = -1;
  }

  Pipe& operator=(Pipe&& pipe) {
    Close();
    pipe_[0] = pipe.pipe_[0];
    pipe_[1] = pipe.pipe_[1];
    pipe.pipe_[0] = pipe.pipe_[1] = -1;
    return *this;
  }

  ~Pipe() {
    Close();
  }

  void Close()      { CloseRead(); CloseWrite(); }
  void CloseRead()  { ClosePipe(0); }
  void CloseWrite() { ClosePipe(1); }

  int ReadFd()  { return pipe_[0]; }
  int WriteFd() { return pipe_[1]; }

  static Pipe Create() {
    Pipe p;
    if (pipe(p.pipe_) == -1) {
      Perror("pipe failed");
    }
    return std::move(p);
  }

  template <typename T>
  void SendValue(const T& t) {
    const char* data = reinterpret_cast<const char*>(&t);
    int size = sizeof(T);
    while (size > 0) {
      const int result = write(WriteFd(), data, size);
      if (result < 0) {
        Perror("write failed");
      } else if (result == 0) {
        Error("write returned 0.");
      }
      size -= result;
      data += result;
    }
  }

  template <typename T>
  bool ReceiveValue(T& t) {
    char* data = reinterpret_cast<char*>(&t);
    int size = sizeof(T);
    while (size > 0) {
      const int result = read(ReadFd(), data, size);
      if (result < 0) {
        Perror("read failed");
      } else if (result == 0) {
        return false;
      }
      data += result;
      size -= result;
    }
    return true;
  }

  bool ReadPortion(std::ostream& stream) {
    const int result = read(ReadFd(), buffer, kBufferSize);
    if (result < 0) {
      Perror("read failed");
    } else if (result == 0) {
      return false;
    }
    stream.write(buffer, result);
    return true;
  }

 private:
  void ClosePipe(int i) {
    if (pipe_[i] != -1) {
      if (close(pipe_[i]) == -1) {
        Perror("close failed");
      }
      pipe_[i] = -1;
    }
  }

  int pipe_[2];
};

pid_t Fork() {
  pid_t pid = fork();
  if (pid == -1) {
    Perror("fork failed");
  }
  return pid;
}

// Waits for a child and returns a non-empty messag when the child failed.
std::string WaitPid(pid_t pid) {
  int status;
  if (waitpid(pid, &status, 0) == -1) {
    Perror("waitpid(", pid, ") failed");
  }
  if (WIFEXITED(status)) {
    if (const int exit_code = WEXITSTATUS(status)) {
      return utils::StrCat("Terminated with exit code ", exit_code, ".");
    } else {
      return {};
    }
  } else if (WIFSIGNALED(status)) {
    const int sig = WTERMSIG(status);
    return utils::StrCat(
        "Terminated by signal ", sig, " (", strsignal(sig), ").");
  } else {
    return utils::StrCat("Died of an unknown cause.");
  }
}

void Dup2(int old_fd, int new_fd) {
  if (dup2(old_fd, new_fd) == -1) {
    Perror("dup2 failed");
  }
}

}  // namespace systm

namespace input_output {

void Flush() {
  if (!std::cout.flush()) {
    Perror("std::cout.flush() failed");
  }
  if (!std::cerr.flush()) {
    Perror("std::cerr.flush() failed");
  }
  std::cerr.flush();
  if (fflush(stdout) == EOF) {
    Perror("fflush(stdout) failed");
  }
  if (fflush(stderr) == EOF) {
    Perror("fflush(stderr) failed");
  }
}

void StderrToDevNull() {
  if (freopen("/dev/null", "w", stderr) == NULL) {
    Perror("freopen(\"/dev/null\", \"w\", stderr) failed");
  }
}

void StdoutToDevNull() {
  if (freopen("/dev/null", "w", stdout) == NULL) {
    Perror("freopen(\"/dev/null\", \"w\", stdout) failed");
  }
}

off_t Tell() {
  const off_t cin_offset = std::cin.tellg();
  if (cin_offset == -1) {
    Perror("std::cin.tellg() failed");
  }
  const off_t stdin_offset = ftell(stdin);
  if (stdin_offset == -1) {
    Perror("ftell(stdin) failed");
  }
  // It's a heuristic.  When I don't know in advance whether the user will
  // use std::cin or scanf, I can only guess that the actual offset is the
  // greater one.
  return std::max(cin_offset, stdin_offset);
}

void Seek(off_t offset) {
  if (!std::cin.seekg(offset)) {
    Perror("std::cin.seekg(", offset, ") failed");
  }
  if (fseek(stdin, offset, SEEK_SET) == -1) {
    Perror("fseek(stdin, ", offset, ", SEEK_SET) failed");
  }
}

}  // namespace input_output

namespace children {

struct ChildData {
  int test_case;
  int test_size;
  pid_t pid;
  systm::Pipe to_child;
  systm::Pipe from_child;
  systm::Pipe output;
};

std::map<pid_t, ChildData> children;

int NumberOfChildren();
ChildData& AddChild(ChildData&& data);
ChildData RemoveChild(pid_t pid);

void ClearChildren();

void KillAllChildrenAndExit();

}  // namespace children

template <typename ...Args>
void Error(Args&& ...args) {
  utils::StrCat_(
      std::cerr, colors::Red, std::forward<Args>(args)..., colors::Reset);
  std::cerr << std::endl;
  children::KillAllChildrenAndExit();
}

namespace state {

enum class JobState {
  kNotStarted,
  kStarted,
  kInputRead,
  kRunning,
  kFinishedOk,
  kFinishedError,
};

constexpr int MinThreads = 1;
constexpr int MaxThreads = 100;
constexpr int MaxNumberOfTests = 100 * 1000 * 1000;

constexpr int DefaultThreads = 1;
constexpr int DefaultMaxThreads = MaxThreads;
constexpr int AllTests = 0;
constexpr int DefaultTest = AllTests;

int threads;
int max_threads;
int test;
int number_of_tests;
std::string input_file;
std::vector<JobState> job_states;

int next_output;
std::vector<std::stringstream> outputs;

off_t next_offset;

std::stringstream& GetOutput(int test_case) {
  return outputs[test_case - 1];
}

std::string JobStateToSymbol(const JobState& state) {
  switch (state) {
    case JobState::kNotStarted:
      return " ";
    case JobState::kStarted:
      return utils::StrCat(colors::Yellow, ".", colors::Reset);
    case JobState::kInputRead:
      return utils::StrCat(colors::Yellow, "?", colors::Reset);
    case JobState::kRunning:
      return utils::StrCat(colors::Blue, "#", colors::Reset);
    case JobState::kFinishedOk:
      return utils::StrCat(colors::Green, "o", colors::Reset);
    case JobState::kFinishedError:
      return utils::StrCat(colors::Red, "x", colors::Reset);
  }
  __builtin_unreachable();
}

void SetState(int test_case, JobState state) {
  job_states[test_case - 1] = state;
  status_bar::UpdateSymbol(test_case - 1, JobStateToSymbol(state));
}

JobState GetState(int test_case) {
  return job_states[test_case - 1];
}

void AppendError(const std::string& error_message) {
  status_bar::PrintMessage(error_message);
}

void ConsumeOutputs() {
  while (next_output <= number_of_tests) {
    const JobState next_state = GetState(next_output);
    if (next_state == JobState::kFinishedOk or
        next_state == JobState::kFinishedError) {
      const std::string output = GetOutput(next_output).str();
      if (!std::cout.write(output.data(), output.size())) {
        Perror("std::cout.write failed");
      }
      next_output++;
    } else {
      break;
    }
  }
}

void PrintStateInfo() {
  std::cerr << colors::Yellow << "Threads = " << threads << " | Max threads = "
            << max_threads << " | Test = "
            << (test == 0 ? utils::StrCat("ALL") : utils::StrCat(test))
            << " | Number of tests = " << number_of_tests
            << " | Input file = '" << input_file << "'"
            << colors::Reset << std::endl;
}

void Initialize(int threads_arg, int max_threads_arg,
                int test_arg, const std::string& input_file_arg) {
  threads = threads_arg;
  max_threads = max_threads_arg;
  test = test_arg;
  input_file = input_file_arg;
  next_output = 1;
  if (!(MinThreads <= threads and threads <= MaxThreads)) {
    throw utils::StrCat(
        "The number of threads should be in range [",
        MinThreads, ", ", MaxThreads, "], but is ", threads, ".");
  }
  if (!(MinThreads <= max_threads and max_threads <= MaxThreads)) {
    throw utils::StrCat(
        "The max number of threads should be in range [",
        MinThreads, ", ", MaxThreads, "], but is ", max_threads, ".");
  }
  if (threads > max_threads) {
    throw utils::StrCat(
        "It doesn't make sense to have the number of threads (", threads,
        ") greater than the max number of threads (", max_threads, ").");
  }
  if (test < 0) {
    throw utils::StrCat(
        "The id of the test cannot be negative, but it is ", test, ".");
  }
  if (access(input_file.c_str(), R_OK) == -1) {
    throw utils::StrCat(
        "The file '", input_file, "' cannot be accessed: ",
        strerror(errno), ".");
  }
  std::ifstream input(input_file);
  if (!input) {
    Perror("std::ifstream() of file '", input_file, "' failed");
  }
  if (!(input >> number_of_tests)) {
    Error("Couldn't read the number of tests from the file '",
          input_file, "'.");
  }
  if (!(0 <= number_of_tests and number_of_tests <= MaxNumberOfTests)) {
    Error("The number of tests should be in range [",
          0, ", ", MaxNumberOfTests, "], but it is ", number_of_tests, ".");
  }
  if (test > number_of_tests) {
    throw utils::StrCat(
        "The test id (", test, ") is greater than the number of tests (",
        number_of_tests, ").");
  }
  next_offset = input.tellg();
  if (next_offset == -1) {
    Perror("input('", input_file, "').tellg() failed");
  }
  job_states.resize(number_of_tests, JobState::kNotStarted);
  outputs.resize(number_of_tests);
}

}  // namespace state

namespace signals {

void InterruptSignalHandler(int sig) {
  children::KillAllChildrenAndExit();
}

class SignalsLocker {
 public:
  SignalsLocker() : is_unlocked_(false) {
    sigset_t full_set;
    if (sigfillset(&full_set) == -1) {
      Perror("sigfillset failed");
    }
    if (sigprocmask(SIG_SETMASK, &full_set, &old_set_) == -1) {
      Perror("sigprocmask failed");
    }
  }

  ~SignalsLocker() {
    Unlock();
  }

  void Unlock() {
    if (!is_unlocked_) {
      if (sigprocmask(SIG_SETMASK, &old_set_, NULL) == -1) {
        Perror("sigprocmask failed");
      }
      is_unlocked_ = true;
    }
  }

  SignalsLocker(const SignalsLocker&) = delete;
  SignalsLocker(SignalsLocker&&) = delete;
  SignalsLocker& operator=(const SignalsLocker&) = delete;
  SignalsLocker& operator=(SignalsLocker&&) = delete;

 private:
  bool is_unlocked_;
  sigset_t old_set_;
};

void SetupSignals() {
  struct sigaction sigint_action;
  sigint_action.sa_handler = InterruptSignalHandler;
  sigint_action.sa_flags = 0;
  if (sigfillset(&sigint_action.sa_mask) == -1) {
    Perror("sigfillset failed");
  }
  if (sigaction(SIGINT, &sigint_action, NULL) == -1) {
    Perror("sigactoin failed");
  }
}

}  // namespace signals

namespace children {

int NumberOfChildren() {
  return children.size();
}

ChildData& AddChild(ChildData&& data) {
  signals::SignalsLocker signals_locker;
  const pid_t pid = data.pid;
  return children.emplace(pid, std::move(data)).first->second;
}

ChildData RemoveChild(pid_t pid) {
  signals::SignalsLocker signals_locker;
  auto it = children.find(pid);
  assert(it != children.end());
  ChildData data = std::move(it->second);
  children.erase(it);
  return std::move(data);
}

void ClearChildren() {
  signals::SignalsLocker signals_locker;
  children.clear();
}

void KillAllChildrenAndExit() {
  for (const auto& child : children) {
    // Checking for errors doesn't make much sense.
    kill(child.first, SIGKILL);
  }
  for (const auto& child : children) {
    waitpid(child.first, NULL, 0);
  }
  _exit(EXIT_FAILURE);
}

}  // namespace children

namespace child_process {

struct ParentData {
  int test_case;
  systm::Pipe to_parent;
  systm::Pipe from_parent;
};

void OpenInput() {
  if (freopen(state::input_file.c_str(), "r", stdin) == NULL) {
    Perror("freopen(\"", state::input_file, "\", \"r\", stdin) failed");
  }
  input_output::Seek(state::next_offset);
}

void Run(ParentData&& data) {
  data.to_parent.CloseRead();
  data.from_parent.CloseWrite();
  OpenInput();
  solution::test_case = data.test_case;
  const int test_size = solution::ReadTest();
  data.to_parent.SendValue(input_output::Tell());
  data.to_parent.SendValue(test_size);
  data.to_parent.Close();
  bool run;
  if (!data.from_parent.ReceiveValue(run)) {
    Error("Receiving @run from the parent failed.");
  }
  data.from_parent.Close();
  if (run) {
    solution::Solve();
  }
}

void Run(int test_case) {
  OpenInput();
  solution::test_case = test_case;
  solution::ReadTest();
  solution::Solve();
}

}  // namespace child_process

namespace main_process {

std::vector<children::ChildData*> RunSomeChildrenAndGatherRunningChildren() {
  assert(!children::children.empty());
  std::vector<children::ChildData*> running_children, waiting_children;
  for (auto& child : children::children) {
    children::ChildData& data = child.second;
    switch (state::GetState(data.test_case)) {
      case state::JobState::kRunning:
        running_children.push_back(&data);
        break;

      case state::JobState::kInputRead:
        waiting_children.push_back(&data);
        break;
    }
  }
  assert(!running_children.empty() or !waiting_children.empty());
  bool is_sorted = false;
  while (running_children.size() < state::threads and
         !waiting_children.empty()) {
    if (!is_sorted) {
      std::sort(waiting_children.begin(), waiting_children.end(),
                [](children::ChildData* a, children::ChildData* b) -> bool {
                  return a->test_size < b->test_size;
                });
      is_sorted = true;
    }
    children::ChildData* child = waiting_children.back();
    waiting_children.pop_back();
    child->to_child.SendValue(true);
    child->to_child.Close();
    running_children.push_back(child);
    state::SetState(child->test_case, state::JobState::kRunning);
  }
  return std::move(running_children);
}

void WaitForChild(pid_t pid) {
  children::ChildData data = children::RemoveChild(pid);
  const std::string error_message = systm::WaitPid(pid);
  if (error_message.empty()) {
    state::SetState(data.test_case, state::JobState::kFinishedOk);
  } else {
    state::SetState(data.test_case, state::JobState::kFinishedError);
    state::AppendError(error_message);
  }
  state::ConsumeOutputs();
}

void WaitForAnyChild() {
  std::vector<children::ChildData*> running_children =
      RunSomeChildrenAndGatherRunningChildren();
  fd_set read_set;
  std::set<pid_t> children_to_terminate;
  while (children_to_terminate.empty()) {
    int max_fd = 0;
    FD_ZERO(&read_set);
    for (children::ChildData* child : running_children) {
      FD_SET(child->output.ReadFd(), &read_set);
      max_fd = std::max(max_fd, child->output.ReadFd());
    }
    const int result = select(max_fd + 1, &read_set, NULL, NULL, NULL);
    if (result == -1) {
      if (errno == EINTR) {
        continue;
      }
      Perror("select failed");
    }
    for (children::ChildData* child : running_children) {
      if (FD_ISSET(child->output.ReadFd(), &read_set)) {
        if (!child->output.ReadPortion(state::GetOutput(child->test_case))) {
          children_to_terminate.insert(child->pid);
        }
      }
    }
  }
  for (const pid_t pid : children_to_terminate) {
    WaitForChild(pid);
  }
}

void RunAllTests() {
  assert(state::test == 0);
  for (int test_case = 1; test_case <= state::number_of_tests; test_case++) {
    while (children::NumberOfChildren() >= state::max_threads) {
      WaitForAnyChild();
    }
    systm::Pipe child_to_parent = systm::Pipe::Create();
    systm::Pipe parent_to_child = systm::Pipe::Create();
    systm::Pipe child_output    = systm::Pipe::Create();
    input_output::Flush();
    signals::SignalsLocker signals_locker;
    pid_t pid = systm::Fork();
    if (pid == 0) {
      signals_locker.Unlock();
      input_output::StderrToDevNull();
      child_output.CloseRead();
      systm::Dup2(child_output.WriteFd(), 1 /* stdout */);
      child_process::ParentData data;
      data.test_case = test_case;
      data.to_parent = std::move(child_to_parent);
      data.from_parent = std::move(parent_to_child);
      child_process::Run(std::move(data));
      std::exit(EXIT_SUCCESS);
    }
    children::ChildData data;
    data.test_case = test_case;
    data.pid = pid;
    data.to_child   = std::move(parent_to_child);
    data.from_child = std::move(child_to_parent);
    data.output     = std::move(child_output);
    data.to_child.CloseRead();
    data.from_child.CloseWrite();
    data.output.CloseWrite();
    children::ChildData& child_data = children::AddChild(std::move(data));
    signals_locker.Unlock();
    state::SetState(test_case, state::JobState::kStarted);
    if (!child_data.from_child.ReceiveValue(state::next_offset)) {
      const std::string error_message = systm::WaitPid(child_data.pid);
      Error("Test case ", test_case, " exited while reading the input. ",
            error_message);
    }
    if (!child_data.from_child.ReceiveValue(child_data.test_size)) {
      const std::string error_message = systm::WaitPid(child_data.pid);
      Error("Test case ", test_case, " exited unexpectedly between sending ",
            "the offset and sending the test size.");
    }
    child_data.from_child.Close();
    state::SetState(test_case, state::JobState::kInputRead);
  }
  while (children::NumberOfChildren() > 0) {
    WaitForAnyChild();
  }
}

void RunSingleTest() {
  assert(state::test > 0);
  for (int test_case = 1; test_case < state::test; test_case++) {
    systm::Pipe child_to_parent = systm::Pipe::Create();
    systm::Pipe parent_to_child = systm::Pipe::Create();
    input_output::Flush();
    signals::SignalsLocker signals_locker;
    pid_t pid = systm::Fork();
    if (pid == 0) {
      signals_locker.Unlock();
      input_output::StderrToDevNull();
      input_output::StdoutToDevNull();
      child_process::ParentData data;
      data.test_case = test_case;
      data.to_parent = std::move(child_to_parent);
      data.from_parent = std::move(parent_to_child);
      child_process::Run(std::move(data));
      std::exit(EXIT_SUCCESS);
    }
    children::ChildData data;
    data.test_case = test_case;
    data.pid = pid;
    children::AddChild(std::move(data));
    signals_locker.Unlock();
    child_to_parent.CloseWrite();
    parent_to_child.CloseRead();
    if (!child_to_parent.ReceiveValue(state::next_offset)) {
      const std::string error_message = systm::WaitPid(pid);
      Error("Test case ", test_case, " exited while reading the input. ",
            error_message);
    }
    int test_size;
    if (!child_to_parent.ReceiveValue(test_size)) {
      const std::string error_message = systm::WaitPid(pid);
      Error("Test case ", test_case, " exited unexpectedly between sending ",
            "the offset and sending the test size.");
    }
    parent_to_child.SendValue(false);
  }
  child_process::Run(state::test);
}

void Run() {
  solution::Init();
  input_output::Flush();
  if (state::test > 0) {
    RunSingleTest();
  } else {
    RunAllTests();
  }
}

}  // namespace main_process

namespace command_line {

struct Arguments {
  int threads     = state::DefaultThreads;
  int max_threads = state::DefaultMaxThreads;
  int test        = state::DefaultTest;
  std::string input_file;
};

int Atoi(const std::string& s) {
  std::stringstream stream(s);
  int result;
  char c;
  if (!(stream >> result) or stream.get(c)) {
    throw utils::StrCat(
        "The value ", s, " is out of bounds [", std::numeric_limits<int>::min(),
        ", ", std::numeric_limits<int>::max(), ".");
  }
  return result;
}

void PrintUsage(int argc, char** argv, int exit_code = EXIT_FAILURE) {
  std::cerr << "Usage: ";
  if (argc == 0) {
    std::cerr << "PROG";
  } else {
    std::cerr << argv[0];
  }
  std::cerr << " input.in [j=<number of threads, default="
            << state::DefaultThreads << ">] [t=<test case, default="
            << state::DefaultTest
            << ">] [m=<max number of started threads, default="
            << state::DefaultMaxThreads << ">]" << std::endl;
  std::exit(exit_code);
}

Arguments Parse(int argc, char** argv) {
  Arguments args;
  std::regex help_regex(R"(^--help$)", std::regex::optimize);
  std::regex threads_regex(R"(^j=([[:digit:]]+)$)", std::regex::optimize);
  std::regex max_threads_regex(R"(^m=([[:digit:]]+)$)", std::regex::optimize);
  std::regex test_regex(R"(^t=([[:digit:]]+)$)", std::regex::optimize);
  for (int i = 1; i < argc; i++) {
    const std::string& arg = argv[i];
    std::smatch match;
    if (std::regex_match(arg, match, help_regex)) {
      PrintUsage(argc, argv, EXIT_SUCCESS);
    } else if (std::regex_match(arg, match, threads_regex)) {
      args.threads = Atoi(match[1]);
    } else if (std::regex_match(arg, match, max_threads_regex)) {
      args.max_threads = Atoi(match[1]);
    } else if (std::regex_match(arg, match, test_regex)) {
      args.test = Atoi(match[1]);
    } else if (args.input_file.empty() and !arg.empty()) {
      args.input_file = arg;
    } else {
      throw utils::StrCat("Unknown argument: '", arg, "'.");
    }
  }
  if (args.input_file.empty()) {
    throw utils::StrCat("The filename of the input file wasn't specified.");
  }
  return args;
}

}  // namespace command_line

int main(int argc, char** argv) {
  signals::SetupSignals();
  try {
    command_line::Arguments args = command_line::Parse(argc, argv);
    state::Initialize(args.threads, args.max_threads, args.test,
                      args.input_file);
  } catch (const std::string& error_message) {
    std::cerr << colors::Red << error_message << colors::Reset << std::endl;
    command_line::PrintUsage(argc, argv);
  }
  state::PrintStateInfo();
  status_bar::Initialize(state::test == 0 /* is_enabled */,
                         state::number_of_tests);
  main_process::Run();
  return EXIT_SUCCESS;
}
