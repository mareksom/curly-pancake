namespace term_codes {

constexpr char Reset[]   = "\033[0m";
constexpr char Red[]     = "\033[31m";
constexpr char Green[]   = "\033[32m";
constexpr char Yellow[]  = "\033[33m";
constexpr char Blue[]    = "\033[34m";
constexpr char Magenta[] = "\033[35m";
constexpr char Cyan[]    = "\033[36m";

constexpr char LightRed[]     = "\033[31m";
constexpr char LightGreen[]   = "\033[32m";
constexpr char LightYellow[]  = "\033[33m";
constexpr char LightBlue[]    = "\033[34m";
constexpr char LightMagenta[] = "\033[35m";
constexpr char LightCyan[]    = "\033[36m";

constexpr char ClearUntilEol[] = "\033[K";

std::string Goto(int row, int col);

}  // namespace term_codes
