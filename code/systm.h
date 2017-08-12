namespace systm {

class Pipe {
 public:
  Pipe();

  Pipe(const Pipe&) = delete;
  Pipe& operator=(const Pipe&) = delete;

  Pipe(Pipe&& pipe);
  Pipe& operator=(Pipe&& pipe);

  ~Pipe();

  void Close();
  void CloseRead();
  void CloseWrite();

  int ReadFd();
  int WriteFd();

  static Pipe Create();

  void SendString(const char* str, std::size_t size);
  bool ReceiveString(char* str, std::size_t size);

  void SendString(const std::string& str);
  bool ReceiveString(std::string& str, std::size_t size);

  template <typename T>
  void SendValue(const T& t);

  template <typename T>
  bool ReceiveValue(T& t);

  // The buffer for the reading is shared between all threads inside a process,
  // so this function cannot be used concurrently.
  bool ReadPortion(std::ostream& stream);

 private:
  void ClosePipe(int i);

  int pipe_[2];
};

pid_t Fork();

// Waits for a child and returns a non-empty message when the child failed.
std::string WaitPid(pid_t pid);

void Dup2(int old_fd, int new_fd);

class InterProcessSemaphore {
 public:
  InterProcessSemaphore() = default;

  InterProcessSemaphore(const InterProcessSemaphore&) = delete;
  InterProcessSemaphore(InterProcessSemaphore&&) = delete;
  InterProcessSemaphore& operator=(const InterProcessSemaphore&) = delete;
  InterProcessSemaphore& operator=(InterProcessSemaphore&&) = delete;

  void Init(int value);

  void V();
  void P();

 private:
  Pipe pipe_;
};

class InterProcessMutex : protected InterProcessSemaphore {
 public:
  void Init();

  void Lock();
  void Unlock();
};

}  // namespace systm
