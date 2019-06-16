namespace signals {

void InterruptSignalHandler(int sig) {
  children::KillAllChildren();
  _exit(EXIT_FAILURE);
}

SignalsLocker::SignalsLocker() : is_unlocked_(false) {
  sigset_t full_set;
  if (sigfillset(&full_set) == -1) {
    Perror("sigfillset failed");
  }
  if (sigprocmask(SIG_SETMASK, &full_set, &old_set_) == -1) {
    Perror("sigprocmask failed");
  }
}

SignalsLocker::~SignalsLocker() {
  Unlock();
}

void SignalsLocker::Unlock() {
  if (!is_unlocked_) {
    if (sigprocmask(SIG_SETMASK, &old_set_, NULL) == -1) {
      Perror("sigprocmask failed");
    }
    is_unlocked_ = true;
  }
}

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
