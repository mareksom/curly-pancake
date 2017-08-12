namespace output_controller {
namespace internal {

int test_cases;
bool enable = false;

std::string JobStateToSymbol(const state::JobState& job_state) {
  switch (job_state) {
    case state::JobState::kNotStarted:
      return " ";
    case state::JobState::kStarted:
      return utils::StrCat(term_codes::Yellow, ".", term_codes::Reset);
    case state::JobState::kInputRead:
      return utils::StrCat(term_codes::Yellow, "?", term_codes::Reset);
    case state::JobState::kRunning:
      return utils::StrCat(term_codes::Blue, "#", term_codes::Reset);
    case state::JobState::kFinishedOk:
      return utils::StrCat(term_codes::Green, "o", term_codes::Reset);
    case state::JobState::kFinishedError:
      return utils::StrCat(term_codes::Red, "x", term_codes::Reset);
  }
  __builtin_unreachable();
}

}  // namespace internal

void Enable(int test_cases) {
  assert(test_cases >= 0);
  internal::test_cases = test_cases;
  internal::enable = true;
  status_bar::Initialize(test_cases);
}

void UpdateProgress(int test_case, double progress) {
}

void UpdateState(int test_case, state::JobState job_state) {
  status_bar::UpdateSymbol(test_case, internal::JobStateToSymbol(job_state));
}

void AddMessage(const std::string& message) {
  status_bar::PrintMessage(message);
}

}  // namespace output_controller
