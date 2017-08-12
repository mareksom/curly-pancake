namespace children {

int NumberOfChildren() {
  return children.size();
}

ChildData& AddChild(ChildData&& data) {
  signals::SignalsLocker signals_locker;
  const pid_t pid = data.pid;
  return children.emplace(pid, std::move(data)).first->second;
}

ChildData RemoveChild(pid_t pid) {
  signals::SignalsLocker signals_locker;
  auto it = children.find(pid);
  assert(it != children.end());
  ChildData data = std::move(it->second);
  children.erase(it);
  return std::move(data);
}

void ClearChildren() {
  signals::SignalsLocker signals_locker;
  children.clear();
}

void KillAllChildrenAndExit() {
  for (const auto& child : children) {
    // Checking for errors doesn't make much sense.
    kill(child.first, SIGKILL);
  }
  for (const auto& child : children) {
    waitpid(child.first, NULL, 0);
  }
  _exit(EXIT_FAILURE);
}

}  // namespace children
