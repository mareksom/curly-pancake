namespace status_bar {

// Must be called at the beginning.
void Initialize(bool is_enabled_arg, int size);

// Updates the status of the i-th test case.
void UpdateSymbol(int i, const std::string& symbol);

// Prints a message above the status.
void PrintMessage(const std::string& message);

}  // namespace status_bar
