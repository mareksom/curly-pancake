namespace tuple_unpacker {
namespace internal {

template <int MaxElement, int NextElement, int ...Sequence>
struct SequenceGenerator :
    SequenceGenerator<MaxElement, NextElement + 1, Sequence..., NextElement> {};

template <int MaxElement, int ...Sequence>
struct SequenceGenerator<MaxElement, MaxElement, Sequence...> {
  template <typename Callback, typename Tuple>
  static auto Call(Callback callback, Tuple&& tuple)
      -> decltype(callback(std::get<Sequence>(std::forward<Tuple>(tuple))...)) {
    return callback(std::get<Sequence>(std::forward<Tuple>(tuple))...);
  }
};

template <typename Tuple>
struct TupleUnpacker {
  using without_reference = typename std::remove_reference<Tuple>::type;
  static constexpr int size = std::tuple_size<without_reference>::value;
  using sequence_generator = SequenceGenerator<size, 0>;

  template <typename Callback>
  static auto Call(Callback&& callback, Tuple&& tuple)
      -> decltype(sequence_generator::Call(std::forward<Callback>(callback),
                                           std::forward<Tuple>(tuple))) {
    return sequence_generator::Call(std::forward<Callback>(callback),
                                    std::forward<Tuple>(tuple));
  }
};

}  // namespace internal

template <typename Callback, typename Tuple>
auto PassUnpackedTuple(Callback&& callback, Tuple&& tuple)
    -> decltype(internal::TupleUnpacker<Tuple>::Call(
                    std::forward<Callback>(callback),
                    std::forward<Tuple>(tuple))) {
  return internal::TupleUnpacker<Tuple>::Call(std::forward<Callback>(callback),
                                              std::forward<Tuple>(tuple));
}

}  // namespace tuple_unpacker
