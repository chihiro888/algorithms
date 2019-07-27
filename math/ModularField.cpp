template<unsigned int mod>
struct ModInt
{
  unsigned int val;
  ModInt() : val(0) {}
  ModInt(const ModInt<mod> &other) : val(other.val) {}
  ModInt(long long x) {
    x %= mod;
    if (x < 0) x += mod;
    val = static_cast<unsigned int>(x);
  }
  unsigned int inverse() const {
    int a = val, b = mod, x = 1, y = 0;
    while (b) {
      int q = a / b;
      int t = a % b;
      a = b;
      b = t;
      int u = x - q * y;
      x = y;
      y = u;
    }
    if (x < 0) x += mod;
    return x;
  }
  ModInt<mod> &operator +=(const ModInt<mod> &other) {
    val += other.val;
    if (val >= mod)
      val -= mod;
    return *this;
  }
  ModInt<mod> &operator -=(const ModInt<mod> &other) {
    val += mod - other.val;
    if (val >= mod)
      val -= mod;
    return *this;
  }
  ModInt<mod> &operator *=(const ModInt<mod> &other) {
    val = (unsigned long long)val * other.val % mod;
    return *this;
  }
  ModInt<mod> &operator /=(const ModInt<mod> &other) {
    val = (unsigned long long)val * other.inverse() % mod;
    return *this;
  }
  ModInt<mod> operator +(const ModInt<mod> &other) const { ModInt<mod> cp(val); cp += other; return cp; }
  ModInt<mod> operator -(const ModInt<mod> &other) const { ModInt<mod> cp(val); cp -= other; return cp; }
  ModInt<mod> operator *(const ModInt<mod> &other) const { ModInt<mod> cp(val); cp *= other; return cp; }
  ModInt<mod> operator /(const ModInt<mod> &other) const { ModInt<mod> cp(val); cp /= other; return cp; }
  bool operator ==(const ModInt<mod> &other) const { return val == other.val; }
  bool operator !=(const ModInt<mod> &other) const { return val != other.val; }
  ModInt<mod> operator +() const { return *this; }
  ModInt<mod> operator -() const { return 0 == val ? 0 : (mod - val); }
  int operator !() const { return !val; }
};
