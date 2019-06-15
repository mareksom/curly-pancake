namespace output_controller {
namespace internal {

int test_cases;
bool enable = false;
std::map<int, double> progress;
std::chrono::time_point<std::chrono::system_clock> beginning_of_time;

constexpr int kFrameTimeMilliseconds = 128;
constexpr int kAnimationSize = 3;

std::vector<std::string> BuildAnimation() {
  std::vector<std::string> color_cycle = {
    term_codes::Red,
    term_codes::LightRed,
    term_codes::Yellow,
    term_codes::LightYellow,
    term_codes::LightCyan,
    term_codes::LightBlue,
    term_codes::Cyan,
    term_codes::Blue,
    term_codes::LightGreen,
    term_codes::Green,
    term_codes::LightMagenta,
    term_codes::Magenta,
  };
  std::string color[2];
  std::vector<std::string> animation;
  auto AddFrame = [&animation, &color](
      const std::string& a, const std::string& b) -> void {
    animation.push_back(
        utils::StrCat(color[0], a, color[1], b, term_codes::Reset));
  };
  int pos[2] = {1, 0};
  auto ShiftColor = [&color_cycle, &pos, &color](int which) -> void {
    pos[which] = (pos[which] + 1) % color_cycle.size();
    color[which] = color_cycle[pos[which]];
  };
  ShiftColor(0);
  ShiftColor(1);
  for (std::size_t i = 0; i < color_cycle.size(); i++) {
    AddFrame(" ,", "'");
    AddFrame(", ", "|");
    ShiftColor(1);
    AddFrame("| ", ",");
    ShiftColor(0);
    AddFrame("'", ", ");
    std::swap(color[0], color[1]);
    AddFrame(",", "' ");
    AddFrame("|", " '");
    AddFrame("'", " |");
    AddFrame(" '", ",");
    std::swap(color[0], color[1]);
  }
  return animation;
}

std::vector<std::string> animation = BuildAnimation();

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
  char percent_str[6];
  sprintf(percent_str, "%3d%%", percent);
  status_bar::SetProgress(percent_str);
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
  status_bar::Initialize(test_cases, internal::kAnimationSize);
  for (int i = 1; i <= test_cases; i++) {
    internal::progress[i] = 0;
  }
  internal::beginning_of_time = std::chrono::system_clock::now();
}

void Update() {
  if (!internal::enable) {
    return;
  }
  auto time = std::chrono::system_clock::now();
  auto duration = time - internal::beginning_of_time;
  auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  const int cycle = internal::animation.size();
  const int frame =
      (milliseconds.count() / internal::kFrameTimeMilliseconds) % cycle;
  status_bar::UpdateSpinner(internal::animation[frame]);
}

void UpdateProgress(int test_case, double progress) {
  if (!internal::enable) {
    return;
  }
  internal::progress[test_case] = progress;
  internal::UpdateProgress();
  Update();
}

void UpdateState(int test_case, state::JobState job_state) {
  if (!internal::enable) {
    return;
  }
  bool progress_changed = false;
  if (job_state == state::JobState::kRunning) {
    auto it = internal::progress.find(test_case);
    if (it == internal::progress.end()) {
      internal::progress[test_case] = 0;
      progress_changed = true;
    }
  } else if (job_state == state::JobState::kFinishedError) {
    internal::progress.erase(test_case);
    progress_changed = true;
  } else if (job_state == state::JobState::kFinishedOk) {
    internal::progress[test_case] = 1;
    progress_changed = true;
  }
  status_bar::UpdateSymbol(test_case - 1,
                           internal::JobStateToSymbol(job_state));
  if (progress_changed) {
    internal::UpdateProgress();
  }
  Update();
}

void AddMessage(const std::string& message) {
  status_bar::PrintMessage(message);
  Update();
}

}  // namespace output_controller
