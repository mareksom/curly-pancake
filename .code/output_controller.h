namespace output_controller {

void Enable(int test_cases);

// A regular update triggered by the clock.
void Update();

void UpdateProgress(int test_case, double progress);
void UpdateState(int test_case, state::JobState job_state);
void AddMessage(const std::string& message);

}  // namespace output_controller
