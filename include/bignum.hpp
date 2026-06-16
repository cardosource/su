#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class BigInt {
public:
    std::vector<uint32_t> digits;
    bool negative = false;
    static constexpr uint32_t BASE = 1'000'000'000u;

    BigInt() : digits{0}, negative{false} {}

    BigInt(long long v){
        negative = v < 0;
        unsigned long long u = negative ? (unsigned long long)(-(v+1))+1 : (unsigned long long)v;
        if(u == 0){ digits.push_back(0); return; }
        while(u){ digits.push_back(u % BASE); u /= BASE; }
    }


    BigInt(const BigInt& other) : digits(other.digits), negative(other.negative) {}
    

    BigInt& operator=(const BigInt& other) {
        if (this != &other) {
            digits = other.digits;
            negative = other.negative;
        }
        return *this;
    }
    

    BigInt(BigInt&& other) noexcept : digits(std::move(other.digits)), negative(other.negative) {}
    
    BigInt& operator=(BigInt&& other) noexcept {
        if (this != &other) {
            digits = std::move(other.digits);
            negative = other.negative;
        }
        return *this;
    }
    

    ~BigInt() = default;

    explicit BigInt(const std::string& s){
        if(s.empty()) { digits.push_back(0); return; }
        size_t i = 0;
        if(s[0] == '-'){ negative = true; i = 1; }
        else if(s[0] == '+'){ i = 1; }

        while(i < s.size()-1 && s[i] == '0') i++;
        for(size_t j = i; j < s.size(); j++)
            if(!isdigit(s[j])) throw std::invalid_argument("BigInt: caracter inválido '"+std::string(1,s[j])+"'");
        int end = (int)s.size();
        while(end > (int)i){
            int start = std::max((int)i, end - 9);
            digits.push_back(std::stoul(s.substr(start, end - start)));
            end = start;
        }
        trim();
        if(isZero()) negative = false;
    }

    bool isZero() const {
        return digits.size() == 1 && digits[0] == 0;
    }

    void trim(){
        while(digits.size() > 1 && digits.back() == 0) digits.pop_back();
    }

    std::string toString() const {
        std::string r;
        if(negative) r += '-';
        r += std::to_string(digits.back());
        for(int i = (int)digits.size()-2; i >= 0; i--){
            std::string chunk = std::to_string(digits[i]);
            r += std::string(9 - chunk.size(), '0') + chunk;
        }
        return r;
    }

    static int cmpAbs(const BigInt& a, const BigInt& b){
        if(a.digits.size() != b.digits.size())
            return a.digits.size() < b.digits.size() ? -1 : 1;
        for(int i = (int)a.digits.size()-1; i >= 0; i--){
            if(a.digits[i] != b.digits[i])
                return a.digits[i] < b.digits[i] ? -1 : 1;
        }
        return 0;
    }

    bool operator==(const BigInt& o) const {
        return negative == o.negative && digits == o.digits;
    }
    bool operator!=(const BigInt& o) const { return !(*this == o); }
    bool operator<(const BigInt& o) const {
        if(negative != o.negative) return negative;
        int c = cmpAbs(*this, o);
        return negative ? c > 0 : c < 0;
    }
    bool operator<=(const BigInt& o) const { return !(o < *this); }
    bool operator>(const BigInt& o)  const { return o < *this; }
    bool operator>=(const BigInt& o) const { return !(*this < o); }

    static BigInt addAbs(const BigInt& a, const BigInt& b){
        BigInt r;
        r.digits.clear();
        uint64_t carry = 0;
        for(size_t i = 0; i < std::max(a.digits.size(), b.digits.size()) || carry; i++){
            uint64_t s = carry;
            if(i < a.digits.size()) s += a.digits[i];
            if(i < b.digits.size()) s += b.digits[i];
            r.digits.push_back(s % BASE);
            carry = s / BASE;
        }
        return r;
    }

    static BigInt subAbs(const BigInt& a, const BigInt& b){
        BigInt r;
        r.digits.clear();
        int64_t borrow = 0;
        for(size_t i = 0; i < a.digits.size(); i++){
            int64_t d = (int64_t)a.digits[i] - borrow
                        - (i < b.digits.size() ? b.digits[i] : 0);
            if(d < 0){ d += BASE; borrow = 1; } else borrow = 0;
            r.digits.push_back((uint32_t)d);
        }
        r.trim();
        return r;
    }

    BigInt operator+(const BigInt& o) const {
        if(negative == o.negative){
            BigInt r = addAbs(*this, o);
            r.negative = negative;
            if(r.isZero()) r.negative = false;
            return r;
        }
        int c = cmpAbs(*this, o);
        if(c == 0) return BigInt(0LL);
        if(c > 0){
            BigInt r = subAbs(*this, o);
            r.negative = negative;
            return r;
        }
        BigInt r = subAbs(o, *this);
        r.negative = o.negative;
        return r;
    }

    BigInt operator-() const {
        BigInt r = *this;
        if(!r.isZero()) r.negative = !r.negative;
        return r;
    }

    BigInt operator-(const BigInt& o) const { return *this + (-o); }

    BigInt operator*(const BigInt& o) const {
        BigInt r;
        r.digits.assign(digits.size() + o.digits.size(), 0);
        for(size_t i = 0; i < digits.size(); i++){
            uint64_t carry = 0;
            for(size_t j = 0; j < o.digits.size() || carry; j++){
                uint64_t cur = (uint64_t)r.digits[i + j] + carry;
                if(j < o.digits.size()) cur += (uint64_t)digits[i] * o.digits[j];
                r.digits[i+j] = cur % BASE;
                carry = cur / BASE;
            }
        }
        r.trim();
        r.negative = negative != o.negative;
        if(r.isZero()) r.negative = false;
        return r;
    }

    static std::pair<BigInt,BigInt> divmod(const BigInt& a, const BigInt& b){
        if(b.isZero()) throw std::runtime_error("BigInt: divisão por zero.");
        BigInt q, rem;
        q.digits.assign(a.digits.size(), 0);
        rem.digits.clear(); rem.digits.push_back(0);
        for(int i = (int)a.digits.size()-1; i >= 0; i--){
            rem.digits.insert(rem.digits.begin(), a.digits[i]);
            rem.trim();
            // encontra maior x tal que b*x <= rem (busca binária)
            uint32_t lo = 0, hi = BASE - 1, x = 0;
            BigInt absB = b; absB.negative = false;
            BigInt absRem = rem; absRem.negative = false;
            while(lo <= hi){
                uint32_t mid = lo + (hi - lo)/2;
                BigInt t = absB * BigInt((long long)mid);
                if(t <= absRem){ x = mid; lo = mid+1; }
                else { if(mid == 0) break; hi = mid-1; }
            }
            q.digits[i] = x;
            BigInt absB2 = b; absB2.negative = false;
            rem = absRem - absB2 * BigInt((long long)x);
        }
        q.trim();
        q.negative = a.negative != b.negative;
        rem.negative = a.negative;
        if(q.isZero()) q.negative = false;
        if(rem.isZero()) rem.negative = false;
        return {q, rem};
    }

    BigInt operator/(const BigInt& o) const { return divmod(*this, o).first; }
    BigInt operator%(const BigInt& o) const { return divmod(*this, o).second; }

    double toDouble() const {
        double r = 0, base = 1;
        for(size_t i = 0; i < digits.size(); i++){
            r += digits[i] * base;
            base *= BASE;
        }
        return negative ? -r : r;
    }
};

class BigFloat {
public:
    BigInt  mantissa;   // dígitos significativos (sem ponto)
    int     exponent;   // deslocamento decimal (pode ser negativo)
    bool    negative;

    static constexpr int DEFAULT_PRECISION = 30; // casas decimais máximas

    BigFloat() : mantissa(0LL), exponent(0), negative(false) {}

    BigFloat(double v){
        negative = v < 0;
        if(negative) v = -v;
        std::ostringstream oss;
        oss.precision(15);
        oss << std::fixed << v;
        *this = fromString((negative ? "-" : "") + oss.str());
    }

    BigFloat(const BigInt& i) : mantissa(i), exponent(0), negative(i.negative) {
        mantissa.negative = false;
    }

    // Copy constructor
    BigFloat(const BigFloat& other) : mantissa(other.mantissa), exponent(other.exponent), negative(other.negative) {}
    
    // Copy assignment
    BigFloat& operator=(const BigFloat& other) {
        if (this != &other) {
            mantissa = other.mantissa;
            exponent = other.exponent;
            negative = other.negative;
        }
        return *this;
    }
    
    // Move constructor
    BigFloat(BigFloat&& other) noexcept : mantissa(std::move(other.mantissa)), exponent(other.exponent), negative(other.negative) {}
    
    // Move assignment
    BigFloat& operator=(BigFloat&& other) noexcept {
        if (this != &other) {
            mantissa = std::move(other.mantissa);
            exponent = other.exponent;
            negative = other.negative;
        }
        return *this;
    }
    
    // Destructor
    ~BigFloat() = default;

    static BigFloat fromString(const std::string& s){
        BigFloat r;
        if(s.empty()) return r;
        size_t i = 0;
        if(s[0] == '-'){ r.negative = true; i = 1; }
        else if(s[0] == '+'){ i = 1; }

        size_t dot = s.find('.', i);
        std::string intPart, fracPart;
        if(dot == std::string::npos){
            intPart  = s.substr(i);
            fracPart = "";
        } else {
            intPart  = s.substr(i, dot - i);
            fracPart = s.substr(dot + 1);
        }
        while(!fracPart.empty() && fracPart.back() == '0') fracPart.pop_back();

        std::string digits = intPart + fracPart;
        size_t start = 0;
        while(start < digits.size()-1 && digits[start] == '0') start++;
        digits = digits.substr(start);

        r.mantissa  = BigInt(digits);
        r.exponent  = -(int)fracPart.size();
        r.negative  = r.negative && !r.mantissa.isZero();
        return r;
    }

    static void align(BigFloat& a, BigFloat& b){
        if(a.exponent < b.exponent){
            // multiplica b.mantissa por 10^(b.exp - a.exp)
            int diff = b.exponent - a.exponent;
            for(int k = 0; k < diff; k++) b.mantissa = b.mantissa * BigInt(10LL);
            b.exponent = a.exponent;
        } else if(b.exponent < a.exponent){
            int diff = a.exponent - b.exponent;
            for(int k = 0; k < diff; k++) a.mantissa = a.mantissa * BigInt(10LL);
            a.exponent = b.exponent;
        }
    }

    BigFloat operator+(const BigFloat& o) const {
        BigFloat a = *this, b = o;
        align(a, b);
        BigFloat r;
        r.exponent = a.exponent;
        BigInt am = a.mantissa; am.negative = a.negative;
        BigInt bm = b.mantissa; bm.negative = b.negative;
        BigInt res = am + bm;
        r.negative  = res.negative;
        r.mantissa  = res; r.mantissa.negative = false;
        return r;
    }

    BigFloat operator-() const {
        BigFloat r = *this;
        if(!r.mantissa.isZero()) r.negative = !r.negative;
        return r;
    }

    BigFloat operator-(const BigFloat& o) const { return *this + (-o); }

    BigFloat operator*(const BigFloat& o) const {
        BigFloat r;
        r.mantissa = mantissa * o.mantissa;
        r.exponent = exponent + o.exponent;
        r.negative = negative != o.negative;
        if(r.mantissa.isZero()) r.negative = false;
        return r;
    }

    BigFloat operator/(const BigFloat& o) const {
        if(o.mantissa.isZero())
            throw std::runtime_error("BigFloat: divisão por zero.");
        BigFloat a = *this;
        int extraScale = DEFAULT_PRECISION - a.exponent + o.exponent;
        if(extraScale > 0)
            for(int k = 0; k < extraScale; k++) a.mantissa = a.mantissa * BigInt(10LL);
        auto [q, rem] = BigInt::divmod(a.mantissa, o.mantissa);
        BigFloat r;
        r.mantissa = q;
        r.exponent = a.exponent - o.exponent - (extraScale > 0 ? extraScale : 0);
        r.negative = negative != o.negative;
        if(r.mantissa.isZero()) r.negative = false;
        return r;
    }

    bool operator==(const BigFloat& o) const {
        BigFloat a = *this, b = o;
        align(a, b);
        return a.negative == b.negative && a.mantissa == b.mantissa;
    }
    bool operator!=(const BigFloat& o) const { return !(*this == o); }
    bool operator<(const BigFloat& o) const {
        if(negative != o.negative) return negative;
        BigFloat a = *this, b = o;
        align(a, b);
        int c = BigInt::cmpAbs(a.mantissa, b.mantissa);
        return negative ? c > 0 : c < 0;
    }
    bool operator<=(const BigFloat& o) const { return !(o < *this); }
    bool operator>(const BigFloat& o)  const { return o < *this; }
    bool operator>=(const BigFloat& o) const { return !(*this < o); }

    std::string toString() const {
        if(mantissa.isZero()) return "0";
        std::string digits = mantissa.toString();
        int len = (int)digits.size();
        int dotPos = len + exponent; 

        std::string r;
        if(negative) r += '-';

        if(dotPos <= 0){
            r += "0.";
            r += std::string(-dotPos, '0');
            r += digits;
        } else if(dotPos >= len){
            r += digits;
        } else {
            r += digits.substr(0, dotPos);
            std::string frac = digits.substr(dotPos);
            while(!frac.empty() && frac.back() == '0') frac.pop_back();
            if(!frac.empty()) r += '.' + frac;
        }
        return r;
    }

    double toDouble() const {
        double r = mantissa.toDouble();
        double scale = 1.0;
        int e = exponent;
        if(e >= 0) while(e--) scale *= 10.0;
        else       while(e++) scale /= 10.0;
        return negative ? -(r * scale) : (r * scale);
    }
};
