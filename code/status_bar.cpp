namespace status_bar {
namespace internal {

bool is_enabled = false;
int spinner_size;
int width, height;
std::string last_spinner;
std::string last_progress;
std::vector<std::string> symbols;

void Goto(int row, int col) {
  if (row <= 0) row += height;
  std::cerr << term_codes::Goto(row, col);
}

void PrintBar() {
  std::cerr << "[";
  for (const std::string& symbol : symbols) {
    std::cerr << symbol;
  }
  std::cerr << "] " << last_spinner << " " << last_progress << std::endl;
}

void PrintOneLineMessage(const std::string& message) {
  assert(is_enabled);
  Goto(-1, 1);
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
  internal::Goto(-1, 2 + i);
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
    if (c == '\n' or last.size() == internal::width) {
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
  internal::Goto(-1, internal::symbols.size() + internal::spinner_size + 5);
  std::cerr << term_codes::ClearUntilEol << progress;
  internal::Goto(0, 1);
}

void UpdateSpinner(const std::string& frame) {
  if (!internal::is_enabled) {
    return;
  }
  internal::last_spinner = frame;
  internal::Goto(-1, internal::symbols.size() + 4);
  std::cerr << frame << std::endl;
  internal::Goto(0, 1);
}

}  // namespace status_bar
