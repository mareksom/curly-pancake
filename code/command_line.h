namespace command_line {

struct Arguments {
  int threads     = state::DefaultThreads;
  int max_threads = state::DefaultMaxThreads;
  int test        = state::DefaultTest;
  std::string input_file;
};

int Atoi(const std::string& s);

void PrintUsage(int argc, char** argv, int exit_code = EXIT_FAILURE);

Arguments Parse(int argc, char** argv);

}  // namespace command_line
