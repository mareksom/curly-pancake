namespace solution {

void UpdateProgress(double progress) {
  if (std::isnan(progress) or progress < 0) {
    progress = 0;
  } else if (progress > 1) {
    progress = 1;
  }
  responder::UpdateProgress(test_case, progress);
}

template <typename ...Args>
void PrintMessage(Args&& ...args) {
  responder::AddMessage(utils::StrCat(std::forward<Args>(args)...));
}

}  // namespace solution
