namespace signals {

void InterruptSignalHandler(int sig);

class SignalsLocker {
 public:
  SignalsLocker();
  ~SignalsLocker();

  void Unlock();

  SignalsLocker(const SignalsLocker&) = delete;
  SignalsLocker(SignalsLocker&&) = delete;
  SignalsLocker& operator=(const SignalsLocker&) = delete;
  SignalsLocker& operator=(SignalsLocker&&) = delete;

 private:
  bool is_unlocked_;
  sigset_t old_set_;
};

void SetupSignals();

}  // namespace signals
