#line 2 "declaration_prefix.cpp"

#include <bits/stdc++.h>

namespace solution {
namespace {

using namespace std;

#define sim template < class c
#define ris return * this
#define dor > debug & operator <<
#define eni(x) sim > typename \
enable_if<sizeof dud<c>(0) x 1, debug&>::type operator<<(c i) {
sim > struct rge { c b, e; };
sim > rge<c> range(c i, c j) { return {i, j}; }
sim > auto dud(c* x) -> decltype(cerr << *x, 0);
sim > char dud(...);
struct debug {
#ifdef LOCAL
~debug() { cerr << endl; }
eni(!=) cerr << boolalpha << i; ris; }
eni(==) ris << range(begin(i), end(i)); }
sim, class b dor(pair < b, c > d) {
  ris << "(" << d.first << ", " << d.second << ")";
}
sim dor(rge<c> d) {
  *this << "[";
  for (c it = d.b; it != d.e; ++it)
    *this << ", " + 2 * (it == d.b) << *it;
  ris << "]";
}
#else
sim dor(const c&) { ris; }
#endif
};
#define imie(x...) " [" #x ": " << (x) << "] "

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

namespace internal {

void StrCat_(std::ostream& stream) {}
template <typename Arg1, typename ...Args>
void StrCat_(std::ostream& stream, Arg1&& arg1, Args&& ...args) {
  stream << std::forward<Arg1>(arg1);
  StrCat_(stream, std::forward<Args>(args)...);
}

template <typename ...Args>
std::string StrCat(Args&& ...args) {
  std::stringstream stream;
  internal::StrCat_(stream, std::forward<Args>(args)...);
  return stream.str();
}

}  // namespace internal

#pragma GCC diagnostic pop

}  // namespace

int test_case;

void UpdateProgress(double progress);
void PrintMessageStr(const std::string& message);

template <typename ...Args>
void PrintMessage(Args&& ...args) {
  PrintMessageStr(internal::StrCat(std::forward<Args>(args)...));
}
