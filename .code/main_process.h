namespace main_process {

std::vector<children::ChildData*> RunSomeChildrenAndGatherRunningChildren();

void WaitForChild(pid_t pid);
void WaitForAnyChild();

void RunAllTests();
void RunSingleTest();

void Run();

}  // namespace main_process
