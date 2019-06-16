namespace responder {

#ifdef DEFINE_CALLBACK
  #error "DEFINE_CALLBACK is already defined."
#endif

#define DEFINE_CALLBACK(Name, ...)  \
    template <typename ...Args>     \
    void Name(Args&& ...args)

DEFINE_CALLBACK(UpdateProgress, int test_case, double progress);
DEFINE_CALLBACK(UpdateState, int test_case, state::JobState job_state);
DEFINE_CALLBACK(AddMessage, std::string message);

#undef DEFINE_CALLBACK

void Start();
void Kill();
void Stop();

}  // namespace responder
