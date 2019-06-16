namespace child_process {

void OpenInput() {
  if (freopen(state::input_file.c_str(), "r", stdin) == NULL) {
    Perror("freopen(\"", state::input_file, "\", \"r\", stdin) failed");
  }
  input_output::Seek(state::next_offset);
}

void Run(ParentData&& data) {
  data.to_parent.CloseRead();
  data.from_parent.CloseWrite();
  OpenInput();
  solution::test_case = data.test_case;
  const int test_size = solution::ReadTest();
  data.to_parent.SendValue(input_output::Tell());
  data.to_parent.SendValue(test_size);
  data.to_parent.Close();
  bool run;
  if (!data.from_parent.ReceiveValue(run)) {
    Error("Receiving @run from the parent failed.");
  }
  data.from_parent.Close();
  if (run) {
    solution::Solve();
  }
}

void Run(int test_case) {
  OpenInput();
  solution::test_case = test_case;
  solution::ReadTest();
  solution::Solve();
}

}  // namespace child_process
