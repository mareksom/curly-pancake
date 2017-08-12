namespace serializer {

class Serializer {
 public:
  template <typename T>
  typename std::enable_if<sizeof solution::dud<T>(0) != 1, Serializer&>::type
  operator<<(const T& t);

  template <typename T>
  typename std::enable_if<sizeof solution::dud<T>(0) != 1, Serializer&>::type
  operator>>(T& t);

  Serializer& operator>>(std::string& str);
  Serializer& operator>>(char* str);

  template <typename T>
  typename std::enable_if<sizeof solution::dud<T>(0) == 1, Serializer&>::type
  operator<<(const T& t);

  template <typename T>
  typename std::enable_if<sizeof solution::dud<T>(0) == 1, Serializer&>::type
  operator>>(T& t);

  template <typename A, typename B>
  Serializer& operator<<(const std::pair<A, B>& p);

  template <typename A, typename B>
  Serializer& operator>>(std::pair<A, B>& p);

  template <typename T>
  Serializer& operator<<(const solution::rge<T>& range);

  template <typename T>
  Serializer& operator>>(solution::rge<T> range);

  void Write(std::ostream& stream);
  void Read(std::istream& stream);

 private:
  void CheckNotEmpty() const;

  std::deque<std::string> out_;
};

template <typename ...Args>
std::string Serialize(Args&& ...args);

template <typename ...Args>
void Deserialize(std::string in, Args& ...args);

}  // namespace serializer
