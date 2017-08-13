namespace main_process {

std::vector<children::ChildData*> RunSomeChildrenAndGatherRunningChildren() {
  assert(!children::children.empty());
  std::vector<children::ChildData*> running_children, waiting_children;
  for (auto& child : children::children) {
    children::ChildData& data = child.second;
    switch (state::GetState(data.test_case)) {
      case state::JobState::kRunning:
        running_children.push_back(&data);
        break;

      case state::JobState::kInputRead:
        waiting_children.push_back(&data);
        break;

      default:
        break;
    }
  }
  assert(!running_children.empty() or !waiting_children.empty());
  bool is_sorted = false;
  while (running_children.size() < state::threads and
         !waiting_children.empty()) {
    if (!is_sorted) {
      std::sort(waiting_children.begin(), waiting_children.end(),
                [](children::ChildData* a, children::ChildData* b) -> bool {
                  return a->test_size < b->test_size;
                });
      is_sorted = true;
    }
    children::ChildData* child = waiting_children.back();
    waiting_children.pop_back();
    child->to_child.SendValue(true);
    child->to_child.Close();
    running_children.push_back(child);
    state::SetState(child->test_case, state::JobState::kRunning);
  }
  return std::move(running_children);
}

void WaitForChild(pid_t pid) {
  children::ChildData data = children::RemoveChild(pid);
  const std::string error_message = systm::WaitPid(pid);
  if (error_message.empty()) {
    state::SetState(data.test_case, state::JobState::kFinishedOk);
  } else {
    state::SetState(data.test_case, state::JobState::kFinishedError);
    responder::AddMessage(
        utils::StrCat(term_codes::Red, error_message, term_codes::Reset));
  }
  state::ConsumeOutputs();
}

void WaitForAnyChild() {
  std::vector<children::ChildData*> running_children =
      RunSomeChildrenAndGatherRunningChildren();
  fd_set read_set;
  std::set<pid_t> children_to_terminate;
  while (children_to_terminate.empty()) {
    int max_fd = 0;
    FD_ZERO(&read_set);
    for (children::ChildData* child : running_children) {
      FD_SET(child->output.ReadFd(), &read_set);
      max_fd = std::max(max_fd, child->output.ReadFd());
    }
    const int result = select(max_fd + 1, &read_set, NULL, NULL, NULL);
    if (result == -1) {
      if (errno == EINTR) {
        continue;
      }
      Perror("select failed");
    }
    for (children::ChildData* child : running_children) {
      if (FD_ISSET(child->output.ReadFd(), &read_set)) {
        if (!child->output.ReadPortion(state::GetOutput(child->test_case))) {
          children_to_terminate.insert(child->pid);
        }
      }
    }
  }
  for (const pid_t pid : children_to_terminate) {
    WaitForChild(pid);
  }
}

void RunAllTests() {
  assert(state::test == 0);
  for (int test_case = 1; test_case <= state::number_of_tests; test_case++) {
    while (children::NumberOfChildren() >= state::max_threads) {
      WaitForAnyChild();
    }
    systm::Pipe child_to_parent = systm::Pipe::Create();
    systm::Pipe parent_to_child = systm::Pipe::Create();
    systm::Pipe child_output    = systm::Pipe::Create();
    input_output::Flush();
    signals::SignalsLocker signals_locker;
    pid_t pid = systm::Fork();
    if (pid == 0) {
      signals_locker.Unlock();
      input_output::StderrToDevNull();
      child_output.CloseRead();
      systm::Dup2(child_output.WriteFd(), 1 /* stdout */);
      child_process::ParentData data;
      data.test_case = test_case;
      data.to_parent = std::move(child_to_parent);
      data.from_parent = std::move(parent_to_child);
      child_process::Run(std::move(data));
      std::exit(EXIT_SUCCESS);
    }
    children::ChildData data;
    data.test_case = test_case;
    data.pid = pid;
    data.to_child   = std::move(parent_to_child);
    data.from_child = std::move(child_to_parent);
    data.output     = std::move(child_output);
    data.to_child.CloseRead();
    data.from_child.CloseWrite();
    data.output.CloseWrite();
    children::ChildData& child_data = children::AddChild(std::move(data));
    signals_locker.Unlock();
    state::SetState(test_case, state::JobState::kStarted);
    if (!child_data.from_child.ReceiveValue(state::next_offset)) {
      const std::string error_message = systm::WaitPid(child_data.pid);
      Error("Test case ", test_case, " exited while reading the input. ",
            error_message);
    }
    if (!child_data.from_child.ReceiveValue(child_data.test_size)) {
      const std::string error_message = systm::WaitPid(child_data.pid);
      Error("Test case ", test_case, " exited unexpectedly between sending ",
            "the offset and sending the test size.");
    }
    child_data.from_child.Close();
    state::SetState(test_case, state::JobState::kInputRead);
  }
  while (children::NumberOfChildren() > 0) {
    WaitForAnyChild();
  }
}

void RunSingleTest() {
  assert(state::test > 0);
  for (int test_case = 1; test_case < state::test; test_case++) {
    systm::Pipe child_to_parent = systm::Pipe::Create();
    systm::Pipe parent_to_child = systm::Pipe::Create();
    input_output::Flush();
    signals::SignalsLocker signals_locker;
    pid_t pid = systm::Fork();
    if (pid == 0) {
      signals_locker.Unlock();
      input_output::StderrToDevNull();
      input_output::StdoutToDevNull();
      child_process::ParentData data;
      data.test_case = test_case;
      data.to_parent = std::move(child_to_parent);
      data.from_parent = std::move(parent_to_child);
      child_process::Run(std::move(data));
      std::exit(EXIT_SUCCESS);
    }
    children::ChildData data;
    data.test_case = test_case;
    data.pid = pid;
    children::AddChild(std::move(data));
    signals_locker.Unlock();
    child_to_parent.CloseWrite();
    parent_to_child.CloseRead();
    if (!child_to_parent.ReceiveValue(state::next_offset)) {
      const std::string error_message = systm::WaitPid(pid);
      Error("Test case ", test_case, " exited while reading the input. ",
            error_message);
    }
    int test_size;
    if (!child_to_parent.ReceiveValue(test_size)) {
      const std::string error_message = systm::WaitPid(pid);
      Error("Test case ", test_case, " exited unexpectedly between sending ",
            "the offset and sending the test size.");
    }
    parent_to_child.SendValue(false);
  }
  child_process::Run(state::test);
}

void Run() {
  solution::Init();
  input_output::Flush();
  if (state::test > 0) {
    RunSingleTest();
  } else {
    RunAllTests();
  }
}

}  // namespace main_process
