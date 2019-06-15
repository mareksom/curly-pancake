#ifndef LOCAL
#pragma GCC optimize ("O3")
#endif

#line 6 ".prefix.cpp"
#include <bits/stdc++.h>

namespace solution {

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

using ld = long double;
using ll = long long;

constexpr int mod = 1000 * 1000 * 1000 + 7;
constexpr int odw2 = (mod + 1) / 2;

void OdejmijOd(int& a, int b) { a -= b; if (a < 0) a += mod; }
int Odejmij(int a, int b) { OdejmijOd(a, b); return a; }
void DodajDo(int& a, int b) { a += b; if (a >= mod) a -= mod; }
int Dodaj(int a, int b) { DodajDo(a, b); return a; }
int Mnoz(int a, int b) { return (ll) a * b % mod; }
void MnozDo(int& a, int b) { a = Mnoz(a, b); }
int Pot(int a, int b) { int res = 1; while (b) { if (b % 2 == 1) MnozDo(res, a); a = Mnoz(a, a); b /= 2; } return res; }
int Odw(int a) { return Pot(a, mod - 2); }
void PodzielDo(int& a, int b) { MnozDo(a, Odw(b)); }
int Podziel(int a, int b) { return Mnoz(a, Odw(b)); }
int Moduluj(ll x) { x %= mod; if (x < 0) x += mod; return x; }

template <typename T> T Maxi(T& a, T b) { return a = max(a, b); }
template <typename T> T Mini(T& a, T b) { return a = min(a, b); }

int test_case;

void UpdateProgress(double progress);

template <typename ...Args>
void PrintMessage(Args&& ...args);
