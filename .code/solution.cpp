namespace solution {

void UpdateProgress(double progress) {
  if (std::isnan(progress) or progress < 0) {
    progress = 0;
  } else if (progress > 1) {
    progress = 1;
  }
  responder::UpdateProgress(test_case, progress);
}

void PrintMessageStr(const std::string& message) {
  responder::AddMessage(message);
}

}  // namespace solution
