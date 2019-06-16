namespace status_bar {
namespace internal {

constexpr int space_in_line = 12;

bool is_enabled = false;
int spinner_size;
int width, height;
int bar_height;
int bar_line_width;
int bar_last_row_cnt;
std::string last_spinner;
std::string last_progress;
std::vector<std::string> symbols;

void Goto(int row, int col) {
  if (row <= 0) row += height;
  std::cerr << term_codes::Goto(row, col);
}

void PrintBar() {
  bar_height = ((int) symbols.size() + width - space_in_line - 1)
                   / (width - space_in_line);
  if (bar_height <= 0) {
    bar_height = 1;
  }
  bar_line_width = (int) symbols.size() / bar_height;
  std::cerr << term_codes::ClearUntilEol << "[";
  int cnt = 0;
  int actual_height = 1;
  for (const std::string& symbol : symbols) {
    if (cnt == bar_line_width) {
      std::cerr << "]\n" << term_codes::ClearUntilEol << "[";
      cnt = 0;
      actual_height++;
    }
    std::cerr << symbol;
    cnt++;
  }
  assert(actual_height == bar_height);
  bar_last_row_cnt = cnt;
  std::cerr << "] " << last_spinner << " " << last_progress << std::endl;
}

void PrintOneLineMessage(const std::string& message) {
  assert(is_enabled);
  Goto(-bar_height, 1);
  std::cerr << term_codes::ClearUntilEol << message << std::endl;
  PrintBar();
}

}  // namespace internal

void Initialize(int size, int spinner_size) {
  winsize w;
  if (isatty(1 /* stdin */) or !isatty(2 /* stderr */) or
      ioctl(STDERR_FILENO, TIOCGWINSZ, &w) == -1) {
    internal::is_enabled = false;
    return;
  }
  internal::is_enabled = true;
  internal::last_spinner = std::string(spinner_size, ' ');
  internal::spinner_size = spinner_size;
  internal::height = w.ws_row;
  internal::width = w.ws_col;
  internal::symbols.resize(size, " ");
  internal::PrintBar();
}

void UpdateSymbol(int i, const std::string& symbol) {
  if (!internal::is_enabled) {
    return;
  }
  assert(0 <= i and i < static_cast<int>(internal::symbols.size()));
  internal::symbols[i] = symbol;
  const int line_num = i / internal::bar_line_width;
  const int line_pos = i % internal::bar_line_width;
  internal::Goto(line_num - internal::bar_height, 2 + line_pos);
  std::cerr << symbol;
  internal::Goto(0, 1);
}

void PrintMessage(const std::string& message) {
  if (!internal::is_enabled) {
    std::cerr << message << std::endl;
    return;
  }
  std::string last;
  for (char c : message) {
    if (c == '\n' or (int) last.size() == internal::width) {
      if (!last.empty()) {
        internal::PrintOneLineMessage(last);
      }
      last.clear();
    }
    if (c != '\n') {
      last.push_back(c);
    }
  }
  if (!last.empty()) {
    internal::PrintOneLineMessage(last);
  }
}

void SetProgress(const std::string& progress) {
  if (!internal::is_enabled) {
    return;
  }
  internal::last_progress = progress;
  internal::Goto(-1, internal::bar_last_row_cnt + internal::spinner_size + 5);
  std::cerr << term_codes::ClearUntilEol << progress;
  internal::Goto(0, 1);
}

void UpdateSpinner(const std::string& frame) {
  if (!internal::is_enabled) {
    return;
  }
  internal::last_spinner = frame;
  internal::Goto(-1, internal::bar_last_row_cnt + 4);
  std::cerr << frame << std::endl;
  internal::Goto(0, 1);
}

}  // namespace status_bar
