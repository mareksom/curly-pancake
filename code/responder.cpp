namespace responder {
namespace internal {

std::mutex mutex;

pid_t pid;
systm::Pipe pipe;
std::map<int, std::function<void(const std::string&)>> callbacks;

int GetCallbackId(std::string function_name) {
  static int next_id = 0;
  static std::map<std::string, int> ids;
  auto it = ids.find(function_name);
  if (it == ids.end()) {
    it = ids.emplace(function_name, next_id++).first;
  }
  return it->second;
}

template <typename ...Args>
std::function<void(Args...)> GenerateCallerFunction(
    std::string function_name, void (*callback_function)(Args...)) {
  const int id = GetCallbackId(function_name);
  return [id](Args ...args) -> void {
    std::string data = serializer::Serialize(args...);
    std::size_t size = data.size();
    std::lock_guard<std::mutex> lock(mutex);
    pipe.SendValue(id);
    pipe.SendValue(size);
    pipe.SendString(data);
  };
}

template <typename ...Args>
void CreateCallback(std::string function_name,
                    void (*callback_function)(Args...)) {
  callbacks[GetCallbackId(function_name)] =
      [callback_function](const std::string& input) -> void {
        std::tuple<Args...> data;
        serializer::Deserialize(input, data);
        tuple_unpacker::PassUnpackedTuple(callback_function, data);
      };
}

#ifdef DEFINE_CALLBACK
  #error "DEFINE_CALLBACK is already defined."
#endif

// Assumes that is inside namespace "internal".
#define DEFINE_CALLBACK(Name, Args...)                                    \
    void Name##_Callback(Args);                                           \
    auto Name##_Caller = GenerateCallerFunction(#Name, Name##_Callback);  \
    }  /* namespace internal */                                           \
    template <typename ...Args2>                                          \
    void UpdateProgress(Args2&& ...args) {                                \
      internal::Name##_Caller(std::forward<Args2>(args)...);              \
    }                                                                     \
    namespace internal {                                                  \
    struct Name##_Struct {                                                \
      Name##_Struct() {                                                   \
        CreateCallback(#Name, Name##_Callback);                           \
      }                                                                   \
    } Name##_Object;                                                      \
    void Name##_Callback(Args)

#undef DEFINE_CALLBACK

void Loop() {
  int id;
  while (pipe.ReceiveValue(id)) {
    std::size_t size;
    std::string input;
    if (!pipe.ReceiveValue(size)) {
      Error("Expected a size of a string.");
    }
    if (!pipe.ReceiveString(input, size)) {
      Error("Expected a string of size ", size, ".");
    }
    auto it = callbacks.find(id);
    if (it == callbacks.end()) {
      Error("Unknown id of a request: ", id, ".");
    }
    it->second(input);
  }
}

}  // namespace internal

void Start() {
  internal::pipe = systm::Pipe::Create();
  internal::pid = systm::Fork();
  if (internal::pid == 0) {
    internal::pipe.CloseWrite();
    internal::Loop();
    internal::pipe.CloseRead();
    std::exit(EXIT_SUCCESS);
  }
  internal::pipe.CloseRead();
}

void Kill() {
  kill(internal::pid, SIGKILL);
  waitpid(internal::pid, NULL, 0);
}

void Stop() {
  internal::pipe.CloseWrite();
  const std::string& error = systm::WaitPid(internal::pid);
  if (!error.empty()) {
    Error("Responder failed: ", error);
  }
}

}  // namespace responder
