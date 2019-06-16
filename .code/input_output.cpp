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
