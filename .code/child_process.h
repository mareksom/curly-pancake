namespace child_process {

struct ParentData {
  int test_case;
  systm::Pipe to_parent;
  systm::Pipe from_parent;
};

void OpenInput();

void Run(ParentData&& data);
void Run(int test_case);

}  // namespace child_process
