
// solution.cpp
#include <bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>
#include "json.hpp" // nlohmann json single-header; put it next to this source or install appropriately

using json = nlohmann::json;
using boost::multiprecision::cpp_int;
using namespace std;

// Utility: convert character to digit value (0..35)
int charToVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    return -1;
}

// Compute gcd for cpp_int
cpp_int abs_cpp(const cpp_int &x) { return x < 0 ? -x : x; }
cpp_int gcd_cpp(cpp_int a, cpp_int b) {
    a = abs_cpp(a); b = abs_cpp(b);
    while (b != 0) {
        cpp_int r = a % b;
        a = b;
        b = r;
    }
    return a;
}

// Rational type with cpp_int numerator/denominator
struct Rational {
    cpp_int num;
    cpp_int den; // always positive
    Rational(): num(0), den(1) {}
    Rational(cpp_int n): num(n), den(1) {}
    Rational(cpp_int n, cpp_int d) {
        if (d == 0) throw runtime_error("Denominator zero");
        if (d < 0) { n = -n; d = -d; }
        cpp_int g = gcd_cpp(abs_cpp(n), d);
        num = n / g;
        den = d / g;
    }
    string toString() const {
        if (den == 1) return num.convert_to<string>();
        return num.convert_to<string>() + "/" + den.convert_to<string>();
    }
};

Rational operator+(const Rational &a, const Rational &b) {
    cpp_int n = a.num*b.den + b.num*a.den;
    cpp_int d = a.den * b.den;
    return Rational(n, d);
}
Rational operator-(const Rational &a, const Rational &b) {
    cpp_int n = a.num*b.den - b.num*a.den;
    cpp_int d = a.den * b.den;
    return Rational(n, d);
}
Rational operator*(const Rational &a, const Rational &b) {
    cpp_int n = a.num * b.num;
    cpp_int d = a.den * b.den;
    return Rational(n, d);
}
Rational operator/(const Rational &a, const Rational &b) {
    if (b.num == 0) throw runtime_error("Division by zero rational");
    cpp_int n = a.num * b.den;
    cpp_int d = a.den * b.num;
    if (d < 0) { n = -n; d = -d; }
    return Rational(n, d);
}

// Convert a string number in base 'base' to cpp_int
cpp_int convertBaseToCppInt(const string &s, int base) {
    if (base < 2 || base > 36) throw runtime_error("Unsupported base");
    cpp_int res = 0;
    for (char c : s) {
        if (c == '+' || c == '-') continue;
        int v = charToVal(c);
        if (v < 0 || v >= base) throw runtime_error("Digit out of range for base");
        res = res * base + v;
    }
    return res;
}

// Lagrange interpolation at x=0 using k points (xi, yi) where xi are integers and yi are integers
Rational lagrangeAtZero(const vector<pair<cpp_int, cpp_int>>& points) {
    int n = (int)points.size();
    Rational result(0);
    for (int i = 0; i < n; ++i) {
        cpp_int xi = points[i].first;
        cpp_int yi = points[i].second;
        Rational term(yi); // yi as rational
        Rational prod(1);
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            cpp_int xj = points[j].first;
            // multiply by (0 - xj) / (xi - xj)
            Rational numer(-xj);
            Rational denom(xi - xj);
            prod = prod * (numer / denom);
        }
        term = term * prod;
        result = result + term;
    }
    return result;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string filename = "input.json";
    if (argc >= 2) filename = argv[1];

    // Read entire file
    ifstream ifs(filename);
    if (!ifs.is_open()) {
        cerr << "Failed to open file: " << filename << "\n";
        return 1;
    }
    json j;
    try {
        ifs >> j;
    } catch (const exception &e) {
        cerr << "JSON parse error: " << e.what() << "\n";
        return 1;
    }

    if (!j.contains("keys") || !j["keys"].contains("k")) {
        cerr << "JSON must contain keys.k\n";
        return 1;
    }
    int k = j["keys"]["k"].get<int>();

    vector<pair<cpp_int, cpp_int>> points;
    for (auto it = j.begin(); it != j.end(); ++it) {
        string key = it.key();
        if (key == "keys") continue;
        // key is x coordinate
        cpp_int x = 0;
        try {
            x = cpp_int(stoll(key));
        } catch (...) {
            x = 0;
            for (char c : key) if (isdigit(c)) x = x*10 + (c - '0');
        }
        json obj = it.value();
        if (!obj.contains("base") || !obj.contains("value")) {
            cerr << "Entry " << key << " missing base/value\n";
            return 1;
        }
        int base = stoi(obj["base"].get<string>());
        string val = obj["value"].get<string>();
        cpp_int y = convertBaseToCppInt(val, base);
        points.emplace_back(x, y);
    }

    // sort by x to have deterministic selection
    sort(points.begin(), points.end(), [](const pair<cpp_int, cpp_int>& a, const pair<cpp_int, cpp_int>& b){
        return a.first < b.first;
    });

    if ((int)points.size() < k) {
        cerr << "Not enough points provided. Have " << points.size() << " need " << k << "\n";
        return 1;
    }

    // Take first k points
    vector<pair<cpp_int, cpp_int>> chosen(points.begin(), points.begin() + k);

    Rational secret = lagrangeAtZero(chosen);
    // Print result (if denominator is 1, print integer)
    if (secret.den == 1) {
        cout << secret.num.convert_to<string>() << "\n";
    } else {
        cout << secret.toString() << "\n";
    }
    return 0;
}
