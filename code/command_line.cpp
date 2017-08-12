namespace command_line {

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

void PrintUsage(int argc, char** argv, int exit_code) {
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
