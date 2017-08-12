namespace status_bar {

// Must be called at the beginning.
void Initialize(int size, int spinner_size);

// Updates the status of the i-th test case.
void UpdateSymbol(int i, const std::string& symbol);

// Prints a message above the status.
void PrintMessage(const std::string& message);

// Sets the message about the overall progress.
void SetProgress(const std::string& progress);

// Sets the frame of the icon for the buffering.
void UpdateSpinner(const std::string& frame);

}  // namespace status_bar
