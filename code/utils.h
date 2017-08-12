namespace utils {

template <typename ...Args>
std::string StrCat(Args&& ...args);

}  // namespace utils

template <typename ...Args>
void Error(Args&& ...args) __attribute__((noreturn));

#ifdef Perror
  #error "Perror is already defined."
#endif

#define Perror(args...)                                        \
    Error("Line ", __LINE__, ": ",                             \
          args, " | error = [", errno, "] ", strerror(errno))
