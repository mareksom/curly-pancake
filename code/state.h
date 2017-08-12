namespace state {

enum class JobState {
  kNotStarted,
  kStarted,
  kInputRead,
  kRunning,
  kFinishedOk,
  kFinishedError,
};

constexpr int MinThreads = 1;
constexpr int MaxThreads = 100;
constexpr int MaxNumberOfTests = 100 * 1000 * 1000;

constexpr int DefaultThreads = 1;
constexpr int DefaultMaxThreads = MaxThreads;
constexpr int AllTests = 0;
constexpr int DefaultTest = AllTests;

int threads;
int max_threads;
int test;
int number_of_tests;
std::string input_file;
std::vector<JobState> job_states;

int next_output;
std::vector<std::stringstream> outputs;

off_t next_offset;

std::stringstream& GetOutput(int test_case);

std::string JobStateToSymbol(const JobState& state);

void SetState(int test_case, JobState state);
JobState GetState(int test_case);

void ConsumeOutputs();

void PrintStateInfo();

void Initialize(int threads_arg, int max_threads_arg,
                int test_arg, const std::string& input_file_arg);

}  // namespace state
