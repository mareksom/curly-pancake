namespace utils {
namespace internal {

void StrCat_(std::ostream& stream) {}
template <typename Arg1, typename ...Args>
void StrCat_(std::ostream& stream, Arg1&& arg1, Args&& ...args) {
  stream << std::forward<Arg1>(arg1);
  StrCat_(stream, std::forward<Args>(args)...);
}

}  // namespace internal

template <typename ...Args>
std::string StrCat(Args&& ...args) {
  std::stringstream stream;
  internal::StrCat_(stream, std::forward<Args>(args)...);
  return stream.str();
}

}  // namespace utils

template <typename ...Args>
void Error(Args&& ...args) {
  utils::internal::StrCat_(std::cerr, term_codes::Red,
                           std::forward<Args>(args)..., term_codes::Reset);
  std::cerr << std::endl;
  children::KillAllChildrenAndExit();
  responder::Kill();
}
