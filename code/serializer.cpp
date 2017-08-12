namespace serializer {
namespace internal {

template <typename T>
struct RemoveConst {
  using value = typename std::remove_const<T>::type;
};

template <typename A, typename B>
struct RemoveConst<std::pair<A, B>> {
  using value = std::pair<typename RemoveConst<A>::value,
                          typename RemoveConst<B>::value>;
};

}  // namespace internal

template <typename T>
typename std::enable_if<sizeof solution::dud<T>(0) != 1, Serializer&>::type
Serializer::operator<<(const T& t) {
  std::stringstream stream;
  stream << t;
  out_.push_back(stream.str());
  return *this;
}

template <typename T>
typename std::enable_if<sizeof solution::dud<T>(0) != 1, Serializer&>::type
Serializer::operator>>(T& t) {
  CheckNotEmpty();
  std::stringstream stream(out_.front());
  stream >> t;
  out_.pop_front();
  return *this;
}

Serializer& Serializer::operator>>(std::string& str) {
  CheckNotEmpty();
  str = std::move(out_.front());
  out_.pop_front();
  return *this;
}

Serializer& Serializer::operator>>(char* str) {
  std::string s;
  *this >> s;
  std::memcpy(str, s.data(), s.size() + 1);
  return *this;
}

template <typename T>
typename std::enable_if<sizeof solution::dud<T>(0) == 1, Serializer&>::type
Serializer::operator<<(const T& t) {
  return *this << range(std::begin(t), std::end(t));
}

template <typename T>
typename std::enable_if<sizeof solution::dud<T>(0) == 1, Serializer&>::type
Serializer::operator>>(T& t) {
  auto inserter = std::inserter(t, std::end(t));
  return *this >> range(inserter, inserter);
}

template <typename A, typename B>
Serializer& Serializer::operator<<(const std::pair<A, B>& p) {
  return *this << p.first << p.second;
}

template <typename A, typename B>
Serializer& Serializer::operator>>(std::pair<A, B>& p) {
  return *this >> p.first >> p.second;
}

template <typename T>
Serializer& Serializer::operator<<(const solution::rge<T>& range) {
  const std::size_t pos = out_.size();
  out_.emplace_back();
  uint64_t size = 0;
  for (auto it = range.b; it != range.e; ++it) {
    size++;
    *this << *it;
  }
  out_[pos] = std::to_string(size);
  return *this;
}

template <typename T>
Serializer& Serializer::operator>>(solution::rge<T> range) {
  uint64_t size = 0;
  *this >> size;
  while (size-- > 0) {
    using ValueType = typename internal::RemoveConst<
        typename T::container_type::value_type>::value;
    ValueType value;
    *this >> value;
    *range.b = std::move(value);
  }
  return *this;
}

void Serializer::Write(std::ostream& stream) {
  for (const std::string& str : out_) {
    uint64_t size = str.size();
    stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
    stream.write(str.data(), size);
  }
  out_.clear();
}

void Serializer::Read(std::istream& stream) {
  constexpr uint64_t kBlockSize = 4096;
  std::vector<char> buffer(kBlockSize + 1, '\0');
  while (stream) {
    uint64_t size;
    std::string str;
    stream.read(reinterpret_cast<char*>(&size), sizeof(size));
    while (stream and size > 0) {
      const uint64_t block = std::min(kBlockSize, size);
      size -= block;
      if (stream.read(buffer.data(), kBlockSize)) {
        str += buffer.data();
      }
    }
  }
}

void Serializer::CheckNotEmpty() const {
  if (out_.empty()) {
    std::cerr << "The serializer doesn't have enough data." << std::endl;
    abort();
  }
}

namespace internal {

void SerializerHelper(Serializer& serializer) {}

template <typename Arg1, typename ...Args>
void SerializerHelper(Serializer& serializer, Arg1&& arg1, Args&& ...args) {
  serializer << std::forward<Arg1>(arg1);
  SerializerHelper(serializer, std::forward<Args>(args)...);
}

void DeserializerHelper(Serializer& serializer) {}

template <typename Arg1, typename ...Args>
void DeserializerHelper(Serializer& serializer, Arg1& arg1, Args& ...args) {
  serializer >> arg1;
  DeserizalizerHelper(serializer, args...);
}

}  // namespace internal

template <typename ...Args>
std::string Serialize(Args&& ...args) {
  Serializer serializer;
  SerializerHelper(serializer, std::forward<Args>(args)...);
  std::stringstream stream;
  serializer.Write(stream);
  return stream.str();
}

template <typename ...Args>
void Deserialize(std::string in, Args& ...args) {
  Serializer serializer;
  std::stringstream stream(std::move(in));
  serializer.Read(stream);
  DeserializerHelper(serializer, args...);
}

}  // namespace serializer
