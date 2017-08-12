namespace status_bar {
namespace internal {

bool is_enabled = false;
int width, height;
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
  std::cerr << "]" << std::endl;
}

}  // namespace internal

void Initialize(int size) {
  winsize w;
  if (isatty(1 /* stdin */) or !isatty(2 /* stderr */) or
      ioctl(STDERR_FILENO, TIOCGWINSZ, &w) == -1) {
    internal::is_enabled = false;
    return;
  }
  internal::is_enabled = true;
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
  internal::Goto(-1, 1);
  for (int i = 0; i < static_cast<int>(internal::symbols.size()) + 2; i++) {
    std::cerr << " ";
  }
  internal::Goto(-1, 1);
  std::cerr << message << std::endl;
  internal::PrintBar();
}

}  // namespace status_bar
