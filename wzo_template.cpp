// int test_case;  // Contains the current test case number.
//
// Informs the master about the progress on this test.  Expects a number in
// range [0, 1], but other values will not crash the program.
// void UpdateProgress(double progress);

void Solve() {
  // Solves the problem.
}

int ReadTest() {
  // Called once per test case.
  // Should read the whole test case, either with std::cin or scanf.
  // Must return the approximate size of the test.  The main process will
  // schedule the largest tests first.  The size can be any integer.
  // The greater the integer, the larger the test is.
}

void Init() {
  // Called once for all threads.  It's a place for any preprocessing.
  // It is forbidden to use stdin in this function.  You may use stdout/stderr
  // - it will appear on the stdout/stderr of the program.
}
