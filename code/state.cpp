namespace state {

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
