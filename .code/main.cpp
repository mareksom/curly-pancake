int main(int argc, char** argv) {
  signals::SetupSignals();
  try {
    command_line::Arguments args = command_line::Parse(argc, argv);
    state::Initialize(args.threads, args.max_threads, args.test,
                      args.input_file);
  } catch (const std::string& error_message) {
    std::cerr << term_codes::Red << error_message << term_codes::Reset
              << std::endl;
    command_line::PrintUsage(argc, argv);
  }
  state::PrintStateInfo();
  if (state::test == 0) {
    output_controller::Enable(state::number_of_tests);
  }
  responder::Start();
  main_process::Run();
  responder::Stop();
  return EXIT_SUCCESS;
}
