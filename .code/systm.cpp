namespace systm {

constexpr int kBufferSize = 4096;
char buffer[kBufferSize];

Pipe::Pipe() : pipe_{-1, -1} {}

Pipe::Pipe(Pipe&& pipe) : pipe_{pipe.pipe_[0], pipe.pipe_[1]} {
  pipe.pipe_[0] = pipe.pipe_[1] = -1;
}

Pipe& Pipe::operator=(Pipe&& pipe) {
  Close();
  pipe_[0] = pipe.pipe_[0];
  pipe_[1] = pipe.pipe_[1];
  pipe.pipe_[0] = pipe.pipe_[1] = -1;
  return *this;
}

Pipe::~Pipe() {
  Close();
}

void Pipe::Close()      { CloseRead(); CloseWrite(); }
void Pipe::CloseRead()  { ClosePipe(0); }
void Pipe::CloseWrite() { ClosePipe(1); }

int Pipe::ReadFd()  { return pipe_[0]; }
int Pipe::WriteFd() { return pipe_[1]; }

Pipe Pipe::Create() {
  Pipe p;
  if (pipe(p.pipe_) == -1) {
    Perror("pipe failed");
  }
  return std::move(p);
}

void Pipe::SendString(const char* str, std::size_t size) {
  while (size > 0) {
    const int result = write(WriteFd(), str, size);
    if (result < 0) {
      Perror("write failed");
    } else if (result == 0) {
      Error("write returned 0.");
    }
    size -= result;
    str += result;
  }
}

bool Pipe::ReceiveString(char* str, std::size_t size) {
  while (size > 0) {
    const int result = read(ReadFd(), str, size);
    if (result < 0) {
      Perror("read failed");
    } else if (result == 0) {
      return false;
    }
    size -= result;
    str += result;
  }
  return true;
}

bool Pipe::ReceiveStringOrTimeout(
    char* str, std::size_t size, bool& timeout, int64_t timeout_milliseconds) {
  fd_set set;
  struct timeval timeval;
  FD_ZERO(&set);
  FD_SET(ReadFd(), &set);
  const int64_t microseconds = timeout_milliseconds * 1000;
  timeval.tv_sec = microseconds / (1000 * 1000);
  timeval.tv_usec = microseconds % (1000 * 1000);
  const int result = select(ReadFd() + 1, &set, nullptr, nullptr, &timeval);
  if (result == -1) {
    Perror("select failed");
  } else if (result == 0) {
    timeout = true;
    return true;
  } else {
    timeout = false;
    return ReceiveString(str, size);
  }
}

void Pipe::SendString(const std::string& str) {
  SendString(str.data(), str.size());
}

bool Pipe::ReceiveString(std::string& str, std::size_t size) {
  str.clear();
  std::vector<char> data(size);
  if (!ReceiveString(data.data(), size)) {
    return false;
  }
  str.insert(str.end(), data.begin(), data.end());
  return true;
}

template <typename T>
void Pipe::SendValue(const T& t) {
  SendString(reinterpret_cast<const char*>(&t), sizeof(T));
}

template <typename T>
bool Pipe::ReceiveValue(T& t) {
  return ReceiveString(reinterpret_cast<char*>(&t), sizeof(T));
}

template <typename T>
bool Pipe::ReceiveValueOrTimeout(T& t, bool& timeout,
                                 int64_t timeout_milliseconds) {
  return ReceiveStringOrTimeout(reinterpret_cast<char*>(&t), sizeof(T), timeout,
                                timeout_milliseconds);
}

bool Pipe::ReadPortion(std::ostream& stream) {
  const int result = read(ReadFd(), buffer, kBufferSize);
  if (result < 0) {
    Perror("read failed");
  } else if (result == 0) {
    return false;
  }
  stream.write(buffer, result);
  return true;
}

void Pipe::ClosePipe(int i) {
  if (pipe_[i] != -1) {
    if (close(pipe_[i]) == -1) {
      Perror("close failed");
    }
    pipe_[i] = -1;
  }
}

pid_t Fork() {
  pid_t pid = fork();
  if (pid == -1) {
    Perror("fork failed");
  }
  return pid;
}

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

void InterProcessSemaphore::Init(int value) {
  constexpr int kPageSize = 4096;
  if (value < 0) {
    Error("The semaphore cannot be negative.");
  }
  if (value >= kPageSize) {
    Error("The semaphore cannot be bigger than the page size (", kPageSize,
          ").");
  }
  pipe_ = Pipe::Create();
  for (int i = 0; i < value; i++) {
    V();
  }
}

void InterProcessSemaphore::V() {
  char value = 0;
  pipe_.SendValue(value);
}

void InterProcessSemaphore::P() {
  char value;
  pipe_.ReceiveValue(value);
}

void InterProcessMutex::Init() {
  InterProcessSemaphore::Init(1);
}

void InterProcessMutex::Lock() {
  P();
}

void InterProcessMutex::Unlock() {
  V();
}

}  // namespace systm
