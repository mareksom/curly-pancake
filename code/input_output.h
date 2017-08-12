namespace input_output {

// Flushes stdin and stderr both with cin/cout and printf/scanf.
void Flush();

// Redirects stderr to /dev/null.
void StderrToDevNull();
// Redirects stdout to /dev/null.
void StdoutToDevNull();

off_t Tell();
void Seek(off_t offset);

}  // namespace input_output
