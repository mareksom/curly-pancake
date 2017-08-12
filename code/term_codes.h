namespace term_codes {

constexpr char Reset[]  = "\033[0m";
constexpr char Red[]    = "\033[31m";
constexpr char Green[]  = "\033[32m";
constexpr char Yellow[] = "\033[33m";
constexpr char Blue[]   = "\033[34m";

constexpr char ClearUntilEol[] = "\033[K";

std::string Goto(int row, int col);

}  // namespace term_codes
