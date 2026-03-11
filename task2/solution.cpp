#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <array>
#include <utility>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>

struct Hobbit {
    std::string name;
    int hp, off, def;

    friend bool operator == (const Hobbit&, const Hobbit&) = default;
};

std::ostream& operator << (std::ostream& out, const Hobbit& h) {
    return out
            << "Hobbit{\"" << h.name << "\", "
            << ".hp=" << h.hp << ", "
            << ".off=" << h.off << ", "
            << ".def=" << h.def << "}";
}

template < typename T >
std::ostream& operator << (std::ostream& out, const std::optional<T>& x) {
    if (!x) return out << "EMPTY_OPTIONAL";
    return out << "Optional{" << *x << "}";
}

#endif

struct HobbitArmy {
    static constexpr bool CHECK_NEGATIVE_HP = true;

    HobbitArmy() = default;

    HobbitArmy(const HobbitArmy& other)
            : root(clone(other.root)) {}

    HobbitArmy(HobbitArmy&& other) noexcept
            : root(other.root) {
        other.root = nullptr;
    }

    HobbitArmy& operator=(HobbitArmy other) noexcept {
        std::swap(root, other.root);
        return *this;
    }

    ~HobbitArmy() {
        destroy(root);
        root = nullptr;
    }

    bool add(const Hobbit& h) {
        if (h.hp <= 0) return false;
        if (find(root, h.name)) return false;
        Node *L, *R;
        split_lt(root, h.name, L, R);
        Node *n = new Node(h);
        root = merge(merge(L, n), R);
        return true;
    }

    std::optional<Hobbit> erase(const std::string& name) {
        Node *L, *midR;
        split_lt(root, name, L, midR);
        Node *M, *R;
        split_le(midR, name, M, R);
        std::optional<Hobbit> out;
        if (M) {
            push(M);
            out = M->val;
            destroy(M);
            M = nullptr;
        }
        root = merge(L, R);
        return out;
    }

    std::optional<Hobbit> stats(const std::string& name) const {
        Node *n = find(root, name);
        if (!n) return std::optional<Hobbit>{};
        return n->val;
    }

    bool enchant(const std::string& first,
                 const std::string& last,
                 int hp_diff,
                 int off_diff,
                 int def_diff) {
        if (first > last) return true;
        Node *A, *B;
        split_lt(root, first, A, B);
        Node *Mid, *C;
        split_le(B, last, Mid, C);
        bool ok = true;
        if (Mid) {
            if (CHECK_NEGATIVE_HP) {
                long long new_min = (long long)Mid->min_hp + (long long)hp_diff;
                if (new_min <= 0) ok = false;
            }
            if (ok) apply_lazy(Mid, hp_diff, off_diff, def_diff);
        }
        root = merge(A, merge(Mid, C));
        return ok;
    }

    void for_each(auto&& fun) const {
        for_each_impl(root, fun);
    }

private:
    struct Node {
        Hobbit   val;
        Node    *l{nullptr};
        Node    *r{nullptr};
        uint32_t prior;
        int      min_hp;
        int      lazy_hp{0};
        int      lazy_off{0};
        int      lazy_def{0};

        explicit Node(const Hobbit& h)
                : val(h),
                  prior(rnd()),
                  min_hp(h.hp) {}
    };

    mutable Node *root = nullptr;

    static uint32_t rnd() {
        static uint32_t x = 0x9e3779b9u;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return x;
    }

    static Node* clone(Node* n) {
        if (!n) return nullptr;
        Node* c = new Node(n->val);
        c->prior    = n->prior;
        c->min_hp   = n->min_hp;
        c->lazy_hp  = n->lazy_hp;
        c->lazy_off = n->lazy_off;
        c->lazy_def = n->lazy_def;
        c->l = clone(n->l);
        c->r = clone(n->r);
        return c;
    }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->l);
        destroy(n->r);
        delete n;
    }

    static void apply_lazy(Node* n, int dh, int doff, int ddef) {
        if (!n) return;
        n->val.hp  += dh;
        n->val.off += doff;
        n->val.def += ddef;
        n->min_hp  += dh;
        n->lazy_hp  += dh;
        n->lazy_off += doff;
        n->lazy_def += ddef;
    }

    static void push(Node* n) {
        if (!n) return;
        if (n->lazy_hp || n->lazy_off || n->lazy_def) {
            apply_lazy(n->l, n->lazy_hp, n->lazy_off, n->lazy_def);
            apply_lazy(n->r, n->lazy_hp, n->lazy_off, n->lazy_def);
            n->lazy_hp = n->lazy_off = n->lazy_def = 0;
        }
    }

    static void pull(Node* n) {
        if (!n) return;
        n->min_hp = n->val.hp;
        if (n->l && n->l->min_hp < n->min_hp) n->min_hp = n->l->min_hp;
        if (n->r && n->r->min_hp < n->min_hp) n->min_hp = n->r->min_hp;
    }

    static void split_lt(Node* cur, const std::string& key, Node*& L, Node*& R) {
        if (!cur) { L = R = nullptr; return; }
        push(cur);
        if (cur->val.name < key) {
            split_lt(cur->r, key, cur->r, R);
            pull(cur);
            L = cur;
        } else {
            split_lt(cur->l, key, L, cur->l);
            pull(cur);
            R = cur;
        }
    }

    static void split_le(Node* cur, const std::string& key, Node*& L, Node*& R) {
        if (!cur) { L = R = nullptr; return; }
        push(cur);
        if (cur->val.name <= key) {
            split_le(cur->r, key, cur->r, R);
            pull(cur);
            L = cur;
        } else {
            split_le(cur->l, key, L, cur->l);
            pull(cur);
            R = cur;
        }
    }

    static Node* merge(Node* L, Node* R) {
        if (!L || !R) return L ? L : R;
        if (L->prior > R->prior) {
            push(L);
            L->r = merge(L->r, R);
            pull(L);
            return L;
        } else {
            push(R);
            R->l = merge(L, R->l);
            pull(R);
            return R;
        }
    }

    static Node* find(Node* cur, const std::string& name) {
        while (cur) {
            push(cur);
            if (name < cur->val.name) cur = cur->l;
            else if (name > cur->val.name) cur = cur->r;
            else return cur;
        }
        return nullptr;
    }

    static void for_each_impl(Node* n, auto& fun) {
        if (!n) return;
        push(n);
        for_each_impl(n->l, fun);
        fun(n->val);
        for_each_impl(n->r, fun);
    }
};


#ifndef __PROGTEST__

////////////////// Dark magic, ignore ////////////////////////

template < typename T >
auto quote(const T& t) { return t; }

std::string quote(const std::string& s) {
    std::string ret = "\"";
    for (char c : s) if (c != '\n') ret += c; else ret += "\\n";
    return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
      std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")" << std::endl; \
      fail++; \
    } else ok++; \
  } while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)


////////////////// End of dark magic ////////////////////////


void check_army(const HobbitArmy& A, const std::vector<Hobbit>& ref, int& ok, int& fail) {
    size_t i = 0;

    A.for_each([&](const Hobbit& h) {
        CHECK(i < ref.size(), true);
        CHECK(h, ref[i]);
        i++;
    });

    CHECK(i, ref.size());
}

void test1(int& ok, int& fail) {
    HobbitArmy A;
    check_army(A, {}, ok, fail);

    CHECK(A.add({"Frodo", 100, 10, 3}), true);
    CHECK(A.add({"Frodo", 200, 10, 3}), false);
    CHECK(A.erase("Frodo"), std::optional(Hobbit("Frodo", 100, 10, 3)));
    CHECK(A.add({"Frodo", 200, 10, 3}), true);

    CHECK(A.add({"Sam", 80, 10, 4}), true);
    CHECK(A.add({"Pippin", 60, 12, 2}), true);
    CHECK(A.add({"Merry", 60, 15, -3}), true);
    CHECK(A.add({"Smeagol", 0, 100, 100}), false);

    if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
        CHECK(A.add({"Smeagol", -100, 100, 100}), false);

    CHECK(A.add({"Smeagol", 200, 100, 100}), true);

    CHECK(A.enchant("Frodo", "Frodo", 10, 1, 1), true);
    CHECK(A.enchant("Sam", "Frodo", -1000, 1, 1), true); // empty range
    CHECK(A.enchant("Bilbo", "Bungo", 1000, 0, 0), true); // empty range

    if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
        CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), false);

    CHECK(A.enchant("Frodo", "Sam", 1, 0, 0), true);
    CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), true);

    CHECK(A.stats("Gandalf"), std::optional<Hobbit>{});
    CHECK(A.stats("Frodo"), std::optional(Hobbit("Frodo", 151, 12, 5)));
    CHECK(A.stats("Merry"), std::optional(Hobbit("Merry", 1, 16, -2)));

    check_army(A, {
            {"Frodo", 151, 12, 5},
            {"Merry", 1, 16, -2},
            {"Pippin", 1, 13, 3},
            {"Sam", 21, 11, 5},
            {"Smeagol", 200, 100, 100},
    }, ok, fail);
}

int main() {
    int ok = 0, fail = 0;
    test1(ok, fail);

    if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
    else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif
