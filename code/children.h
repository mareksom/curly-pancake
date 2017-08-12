namespace children {

struct ChildData {
  int test_case;
  int test_size;
  pid_t pid;
  systm::Pipe to_child;
  systm::Pipe from_child;
  systm::Pipe output;
};

std::map<pid_t, ChildData> children;

int NumberOfChildren();
ChildData& AddChild(ChildData&& data);
ChildData RemoveChild(pid_t pid);

void ClearChildren();

void KillAllChildrenAndExit();

}  // namespace children
