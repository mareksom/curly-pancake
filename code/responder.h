namespace responder {

#ifdef DEFINE_CALLBACK
  #error "DEFINE_CALLBACK is already defined."
#endif

#define DEFINE_CALLBACK(Name, ...)  \
    template <typename ...Args>     \
    void Name(Args&& ...args);

#undef DEFINE_CALLBACK

void Start();
void Kill();
void Stop();

}  // namespace responder
