namespace term_codes {

std::string Goto(int row, int col) {
  return utils::StrCat("\033[", row, ";", col, "H");
}

}  // namespace term_codes
