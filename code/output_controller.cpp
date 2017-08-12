namespace output_controller {
namespace internal {

int test_cases;
bool enable = false;
std::map<int, double> progress;

void UpdateProgress() {
  int cnt = 0;
  double progress_sum = 0;
  for (const auto& it : progress) {
    cnt++;
    progress_sum += it.second;
  }
  double overall_progress;
  if (cnt == 0) {
    overall_progress = 1;
  } else {
    overall_progress = progress_sum / cnt;
  }
  const int percent = std::round(overall_progress * 100);
  status_bar::SetProgress(utils::StrCat(percent, "%"));
}

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
  internal::progress[test_case] = progress;
  internal::UpdateProgress();
}

void UpdateState(int test_case, state::JobState job_state) {
  bool progress_changed = false;
  if (job_state == state::JobState::kRunning) {
    auto it = internal::progress.find(test_case);
    if (it == internal::progress.end()) {
      internal::progress[test_case] = 0;
      progress_changed = true;
    }
  } else if (job_state == state::JobState::kFinishedOk or
             job_state == state::JobState::kFinishedError) {
    internal::progress.erase(test_case);
    progress_changed = true;
  }
  status_bar::UpdateSymbol(test_case - 1,
                           internal::JobStateToSymbol(job_state));
  if (progress_changed) {
    internal::UpdateProgress();
  }
}

void AddMessage(const std::string& message) {
  status_bar::PrintMessage(message);
}

}  // namespace output_controller
