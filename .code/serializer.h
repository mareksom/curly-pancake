namespace serializer {

class Serializer {
 public:
  template <typename T>
  typename std::enable_if<sizeof solution::dud<T>(0) != 1 and
                          !std::is_enum<T>::value, Serializer&>::type
  operator<<(const T& t);

  template <typename T>
  typename std::enable_if<sizeof solution::dud<T>(0) != 1 and
                          !std::is_enum<T>::value, Serializer&>::type
  operator>>(T& t);

  Serializer& operator>>(std::string& str);
  Serializer& operator>>(char* str);

  template <typename T>
  typename std::enable_if<sizeof solution::dud<T>(0) == 1 and
                          !std::is_enum<T>::value, Serializer&>::type
  operator<<(const T& t);

  template <typename T>
  typename std::enable_if<sizeof solution::dud<T>(0) == 1 and
                          !std::is_enum<T>::value, Serializer&>::type
  operator>>(T& t);

  template <typename T>
  typename std::enable_if<std::is_enum<T>::value, Serializer&>::type operator<<(
      const T& t);

  template <typename T>
  typename std::enable_if<std::is_enum<T>::value, Serializer&>::type operator>>(
      T& t);

  template <typename A, typename B>
  Serializer& operator<<(const std::pair<A, B>& p);

  template <typename A, typename B>
  Serializer& operator>>(std::pair<A, B>& p);

  template <typename ...Args>
  Serializer& operator<<(const std::tuple<Args...>& t);

  template <typename ...Args>
  Serializer& operator>>(std::tuple<Args...>& t);

  template <typename T>
  Serializer& operator<<(const solution::rge<T>& range);

  template <typename T>
  Serializer& operator>>(solution::rge<T> range);

  void Write(std::ostream& stream);
  void Read(std::istream& stream);

 private:
  void CheckNotEmpty() const;

  template <typename ...Args>
  Serializer& PutMany(Args&& ...args);

  template <typename ...Args>
  Serializer& GetMany(Args&& ...args);

  std::deque<std::string> out_;
};

template <typename ...Args>
std::string Serialize(Args&& ...args);

template <typename ...Args>
void Deserialize(std::string in, Args& ...args);

}  // namespace serializer
