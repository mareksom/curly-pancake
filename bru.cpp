// int test_case;  // Contains the current test case number.

int liczba;

void Solve() {
  // Solves the problem.
  std::this_thread::sleep_for(1s);
  for (int i = 1; i < liczba; i++) {
    UpdateProgress((double) i / liczba);
    std::this_thread::sleep_for(1s);
    if (test_case == 7) {
      PrintMessage("Jestem siedem, i = ", i, ".");
    }
  }
  printf("%d\n", liczba);
  debug() << imie(test_case);
}

int ReadTest() {
  // Called once per test case.
  // Should read the whole test case, either with std::cin or scanf.
  // Must return the approximate size of the test.  The main process will
  // schedule the largest tests first.  The size can be any integer.
  // The greater the integer, the larger the test is.
  scanf("%d", &liczba);
  return liczba;
}

void Init() {
  // Called once for all threads.  It's a place for any preprocessing.
  // It is forbidden to use stdin in this function.  You may use stdout/stderr
  // - it will appear on the stdout/stderr of the program.
  //debug() << "Swyg";
}
