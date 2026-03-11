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
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <compare>

struct Vault {
  unsigned value;
  unsigned pebbles;
  std::vector<std::pair<unsigned, unsigned>> missing_connections;
};

struct UnlockingSequence {
  unsigned vault_id;
  std::vector<bool> moved_pebbles;
};

#endif

struct DSU {
  std::vector<unsigned> p, r;
  explicit DSU(unsigned n = 0) : p(n), r(n, 0) {
    for (unsigned i = 0; i < n; ++i) p[i] = i;
  }
  unsigned find(unsigned x) {
    while (p[x] != x) {
      p[x] = p[p[x]];
      x = p[x];
    }
    return x;
  }
  void unite(unsigned a, unsigned b) {
    a = find(a); b = find(b);
    if (a == b) return;
    if (r[a] < r[b]) std::swap(a, b);
    p[b] = a;
    if (r[a] == r[b]) ++r[a];
  }
};

struct HeistItem {
  unsigned id;
  unsigned value;
  unsigned weight;
};

static bool any_bit_in_range(const std::vector<std::uint64_t>& bits, unsigned L, unsigned R) {
  if (L > R) return false;
  const unsigned wl = L / 64u, bl = L % 64u;
  const unsigned wr = R / 64u, br = R % 64u;

  if (wl == wr) {
    std::uint64_t maskL = (~0ull) << bl;
    std::uint64_t maskR = (br == 63u) ? ~0ull : ((1ull << (br + 1u)) - 1ull);
    return (bits[wl] & maskL & maskR) != 0ull;
  }

  {
    std::uint64_t mask = (~0ull) << bl;
    if ((bits[wl] & mask) != 0ull) return true;
  }

  for (unsigned w = wl + 1u; w < wr; ++w)
    if (bits[w] != 0ull) return true;

  {
    std::uint64_t mask = (br == 63u) ? ~0ull : ((1ull << (br + 1u)) - 1ull);
    if ((bits[wr] & mask) != 0ull) return true;
  }

  return false;
}

static void bitset_or_shift(std::vector<std::uint64_t>& bits, unsigned shift) {
  if (shift == 0u) return;
  const unsigned wshift = shift / 64u;
  const unsigned bshift = shift % 64u;
  const int W = static_cast<int>(bits.size());

  for (int i = W - 1; i >= 0; --i) {
    std::uint64_t v = 0ull;
    const int src = i - static_cast<int>(wshift);
    if (src >= 0) {
      v = bits[static_cast<size_t>(src)] << bshift;
      if (bshift != 0u && src - 1 >= 0) {
        v |= bits[static_cast<size_t>(src - 1)] >> (64u - bshift);
      }
    }
    bits[static_cast<size_t>(i)] |= v;
  }
}

static bool compute_unlock(const Vault& v, std::vector<bool>* movedOut) {
  const unsigned n = v.pebbles;
  if ((n & 1u) != 0u) return false;
  const unsigned half = n / 2u;

  if (n == 0u) {
    if (movedOut) movedOut->clear();
    return true;
  }

  const auto& edges = v.missing_connections;
  if (edges.empty()) {
    if (!movedOut) return true;
    movedOut->assign(n, false);
    for (unsigned i = 0; i < half; ++i) (*movedOut)[i] = true;
    return true;
  }

  static thread_local std::vector<unsigned> parent;
  static thread_local std::vector<unsigned> rnk;
  static thread_local std::vector<unsigned> initStamp;
  static thread_local unsigned curStamp = 1u;
  static thread_local std::vector<unsigned> touched;

  static thread_local std::vector<int> rootToComp;
  static thread_local std::vector<unsigned> compStamp;

  static thread_local std::vector<unsigned> compSize;
  static thread_local std::vector<unsigned> bigRoots;

  static thread_local std::vector<unsigned> compOfNode;
  static thread_local std::vector<std::uint64_t> bits;

  auto bump_stamp = [&]() {
    ++curStamp;
    if (curStamp == 0u) {
      curStamp = 1u;
      std::fill(initStamp.begin(), initStamp.end(), 0u);
      std::fill(compStamp.begin(), compStamp.end(), 0u);
    }
  };

  if (parent.size() < n) {
    parent.resize(n);
    rnk.resize(n);
    initStamp.resize(n, 0u);
    rootToComp.resize(n);
    compStamp.resize(n, 0u);
    if (movedOut) compOfNode.resize(n);
  } else {
    if (movedOut && compOfNode.size() < n) compOfNode.resize(n);
  }

  bump_stamp();
  touched.clear();
  touched.reserve(edges.size() * 2u);

  auto ensure = [&](unsigned x) {
    if (initStamp[x] != curStamp) {
      initStamp[x] = curStamp;
      parent[x] = x;
      rnk[x] = 0u;
      touched.push_back(x);
    }
  };

  auto findp = [&](unsigned x) {
    while (parent[x] != x) {
      parent[x] = parent[parent[x]];
      x = parent[x];
    }
    return x;
  };

  auto unitep = [&](unsigned a, unsigned b) {
    a = findp(a); b = findp(b);
    if (a == b) return;
    if (rnk[a] < rnk[b]) std::swap(a, b);
    parent[b] = a;
    if (rnk[a] == rnk[b]) ++rnk[a];
  };

  for (const auto& e : edges) {
    unsigned u = e.first, w = e.second;
    if (u >= n || w >= n) continue;
    ensure(u);
    ensure(w);
    unitep(u, w);
  }

  const unsigned touchedCnt = static_cast<unsigned>(touched.size());
  unsigned singles = n - touchedCnt;

  compSize.clear();
  bigRoots.clear();
  compSize.reserve(touchedCnt);
  bigRoots.reserve(touchedCnt);

  for (unsigned x : touched) {
    unsigned root = findp(x);
    if (compStamp[root] != curStamp) {
      compStamp[root] = curStamp;
      rootToComp[root] = static_cast<int>(compSize.size());
      compSize.push_back(0u);
    }
    unsigned cid = static_cast<unsigned>(rootToComp[root]);
    compSize[cid] += 1u;
    if (movedOut) compOfNode[x] = cid;
  }

  for (unsigned cid = 0; cid < compSize.size(); ++cid) {
    unsigned sz = compSize[cid];
    if (sz == 1u) {
      singles += 1u;
    } else if (sz >= 2u) {
      bigRoots.push_back(cid);
    }
  }

  if (singles >= half) {
    if (!movedOut) return true;

    movedOut->assign(n, false);
    unsigned need = half;

    for (unsigned x : touched) {
      if (need == 0u) break;
      unsigned cid = compOfNode[x];
      if (compSize[cid] == 1u && !(*movedOut)[x]) {
        (*movedOut)[x] = true;
        --need;
      }
    }

    for (unsigned i = 0; i < n && need > 0u; ++i) {
      if (initStamp[i] != curStamp) {
        (*movedOut)[i] = true;
        --need;
      }
    }

    return need == 0u;
  }

  const unsigned L = half - singles;

  if (!movedOut) {
    const unsigned words = (half + 64u) / 64u;
    bits.assign(words, 0ull);
    bits[0] = 1ull;

    for (unsigned cid : bigRoots) {
      unsigned sz = compSize[cid];
      if (sz > half) continue;
      bitset_or_shift(bits, sz);
      if (any_bit_in_range(bits, L, half)) return true;
    }
    return any_bit_in_range(bits, L, half);
  }

  std::vector<int> parentDP(half + 1u, -1);
  std::vector<int> prevSum(half + 1u, -1);
  parentDP[0] = -2;

  for (unsigned j = 0; j < bigRoots.size(); ++j) {
    unsigned cid = bigRoots[j];
    unsigned sz = compSize[cid];
    if (sz > half) continue;

    for (unsigned s = half; s >= sz; --s) {
      if (parentDP[s] == -1 && parentDP[s - sz] != -1) {
        parentDP[s] = static_cast<int>(j);
        prevSum[s] = static_cast<int>(s - sz);
      }
      if (s == sz) break;
    }
  }

  int target = -1;
  for (unsigned s = half; s >= L; --s) {
    if (parentDP[s] != -1) { target = static_cast<int>(s); break; }
    if (s == 0u) break;
  }
  if (target == -1) return false;

  std::vector<char> chosenBig(bigRoots.size(), 0);
  unsigned sumBig = static_cast<unsigned>(target);

  for (unsigned cur = sumBig; cur > 0u; ) {
    int j = parentDP[cur];
    chosenBig[static_cast<size_t>(j)] = 1;
    cur = static_cast<unsigned>(prevSum[cur]);
  }

  std::vector<char> chosenComp(compSize.size(), 0);
  for (size_t j = 0; j < bigRoots.size(); ++j)
    if (chosenBig[j]) chosenComp[bigRoots[j]] = 1;

  movedOut->assign(n, false);

  for (unsigned x : touched) {
    unsigned cid = compOfNode[x];
    if (chosenComp[cid]) (*movedOut)[x] = true;
  }

  unsigned needSingles = half - sumBig;

  for (unsigned x : touched) {
    if (needSingles == 0u) break;
    unsigned cid = compOfNode[x];
    if (!(*movedOut)[x] && compSize[cid] == 1u) {
      (*movedOut)[x] = true;
      --needSingles;
    }
  }

  for (unsigned i = 0; i < n && needSingles > 0u; ++i) {
    if (initStamp[i] != curStamp) {
      (*movedOut)[i] = true;
      --needSingles;
    }
  }

  return needSingles == 0u;
}

static std::vector<unsigned> choose_knapsack_persistent(
  const std::vector<HeistItem>& items,
  unsigned capacity
) {
  struct Node { int prev; int itemIdx; };

  const unsigned C = capacity;
  std::vector<long long> best(C + 1u, -1);
  std::vector<int> head(C + 1u, -1);
  std::vector<Node> nodes;

  best[0] = 0;

  for (size_t idx = 0; idx < items.size(); ++idx) {
    const unsigned w = items[idx].weight;
    const long long val = static_cast<long long>(items[idx].value);
    if (w == 0u || w > C) continue;

    for (unsigned t = C; t >= w; --t) {
      const long long base = best[t - w];
      if (base != -1) {
        const long long cand = base + val;
        if (cand > best[t]) {
          best[t] = cand;
          nodes.push_back(Node{ head[t - w], static_cast<int>(idx) });
          head[t] = static_cast<int>(nodes.size() - 1u);
        }
      }
      if (t == w) break;
    }
  }

  unsigned bestT = 0u;
  for (unsigned t = 1u; t <= C; ++t)
    if (best[t] > best[bestT]) bestT = t;

  std::vector<unsigned> chosen;
  for (int h = head[bestT]; h != -1; h = nodes[static_cast<size_t>(h)].prev) {
    int idx = nodes[static_cast<size_t>(h)].itemIdx;
    chosen.push_back(items[static_cast<size_t>(idx)].id);
  }
  std::reverse(chosen.begin(), chosen.end());
  return chosen;
}

std::vector<UnlockingSequence> plan_heist(
  const std::vector<Vault>& vaults,
  unsigned transition_time,
  unsigned max_time
) {
  std::vector<UnlockingSequence> res;

  auto build_seq = [&](unsigned id) -> std::optional<UnlockingSequence> {
    std::vector<bool> moved;
    if (!compute_unlock(vaults[id], &moved)) return std::nullopt;
    return UnlockingSequence{ id, std::move(moved) };
  };

  if (transition_time > max_time) {
    unsigned bestId = 0u;
    bool found = false;

    for (unsigned i = 0; i < vaults.size(); ++i) {
      const Vault& v = vaults[i];
      if ((v.pebbles & 1u) != 0u) continue;
      const unsigned unlock = v.pebbles / 2u;
      if (unlock > max_time) continue;
      if (!compute_unlock(v, nullptr)) continue;

      if (!found || v.value > vaults[bestId].value) {
        bestId = i;
        found = true;
      }
    }

    if (!found) return res;
    auto seq = build_seq(bestId);
    if (seq) res.push_back(std::move(*seq));
    return res;
  }

  const unsigned capacity = max_time + transition_time;

  std::vector<unsigned> chosenZero;
  std::vector<std::vector<std::pair<unsigned, unsigned>>> byW(capacity + 1u);

  for (unsigned i = 0; i < vaults.size(); ++i) {
    const Vault& v = vaults[i];
    if ((v.pebbles & 1u) != 0u) continue;

    const unsigned unlock = v.pebbles / 2u;
    if (unlock > max_time) continue;

    if (!compute_unlock(v, nullptr)) continue;

    const unsigned w = unlock + transition_time;
    if (w == 0u) {
      chosenZero.push_back(i);
      continue;
    }
    if (w > capacity) continue;

    byW[w].push_back(std::make_pair(v.value, i));
  }

  std::vector<HeistItem> items;
  items.reserve(capacity * 2u);

  for (unsigned w = 1u; w <= capacity; ++w) {
    auto& vec = byW[w];
    if (vec.empty()) continue;

    const unsigned k = capacity / w;
    if (k == 0u) continue;

    if (vec.size() > static_cast<size_t>(k)) {
      std::nth_element(
        vec.begin(),
        vec.begin() + static_cast<std::ptrdiff_t>(k),
        vec.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; }
      );
      vec.resize(static_cast<size_t>(k));
    }

    for (const auto& pr : vec) {
      unsigned id = pr.second;
      items.push_back(HeistItem{ id, vaults[id].value, w });
    }
  }

  std::vector<unsigned> chosenPos = choose_knapsack_persistent(items, capacity);

  std::vector<unsigned> chosenIds;
  chosenIds.reserve(chosenZero.size() + chosenPos.size());
  chosenIds.insert(chosenIds.end(), chosenZero.begin(), chosenZero.end());
  chosenIds.insert(chosenIds.end(), chosenPos.begin(), chosenPos.end());

  res.reserve(chosenIds.size());
  for (unsigned id : chosenIds) {
    auto seq = build_seq(id);
    if (seq) res.push_back(std::move(*seq));
  }

  return res;
}


#ifndef __PROGTEST__

struct TestFailed : std::runtime_error {
  using std::runtime_error::runtime_error;
};

#define CHECK(cond, msg) do { \
    if (!(cond)) throw TestFailed(msg); \
  } while (0)

void check_unlocking_sequence(
  unsigned pebbles,
  const std::vector<std::pair<unsigned, unsigned>>& missing_connections,
  const std::vector<bool>& moved
) {
  CHECK(moved.size() == pebbles, "Solution has wrong size.\n");

  size_t moved_cnt = 0;
  for (bool p : moved) moved_cnt += p;
  CHECK(2*moved_cnt == pebbles,
    "Exactly half of the pebbles must be moved.\n");

  for (auto [ u, v ] : missing_connections) CHECK(moved[u] == moved[v],
    "Pebble not connected with all on other side.\n");
}

void check_solution(
  const std::vector<UnlockingSequence>& solution,
  unsigned expected_value,
  const std::vector<Vault>& vaults,
  unsigned transition_time,
  unsigned max_time
) {
  unsigned time = 0, value = 0;
  std::vector<bool> robbed(vaults.size(), false);

  for (size_t i = 0; i < solution.size(); i++) {
    const auto& [ id, moved ] = solution[i];

    CHECK(id < vaults.size(), "Id is out of range.\n");
    CHECK(!robbed[id], "Robbed same vault twice.\n");
    robbed[id] = true;

    const auto& vault = vaults[id];
    value += vault.value;

    if (i != 0) time += transition_time;
    time += vault.pebbles / 2;
    CHECK(time <= max_time, "Run out of time.\n");

    check_unlocking_sequence(vault.pebbles, vault.missing_connections, moved);
  }

  CHECK(value == expected_value, "Total value mismatch.\n");
}


struct Test {
  unsigned expected_value;
  unsigned max_time;
  unsigned transition_time;
  std::vector<Vault> vaults;
};

inline const std::vector<Test> TESTS = {
  Test{
    .expected_value = 3010, .max_time = 3, .transition_time = 8,
    .vaults = {
      { .value = 3010, .pebbles = 6, .missing_connections = { {3,4}, {0,1}, {4,5}, {5,3}, } },
      { .value = 3072, .pebbles = 6, .missing_connections = { {2,1}, {1,3}, {0,1}, {0,3}, {4,5}, {2,3}, } },
      { .value = 5069, .pebbles = 10, .missing_connections = { {7,2}, {3,4}, {0,1}, {8,4}, {1,2}, {8,3}, {7,0}, {5,6}, {9,5}, {9,6}, } },
      { .value = 2061, .pebbles = 4, .missing_connections = { {3,0}, {2,1}, {0,2}, {1,3}, } },
    }
  },
  Test{
    .expected_value = 6208, .max_time = 13, .transition_time = 12,
    .vaults = {
      { .value = 6011, .pebbles = 12, .missing_connections = { {1,5}, {2,4}, {5,10}, {1,10}, {0,3}, {8,3}, {8,0}, {9,8}, {2,6}, {3,9}, {0,9}, {4,6}, {11,7}, } },
      { .value = 2056, .pebbles = 4, .missing_connections = { {1,0}, {2,0}, {2,1}, } },
      { .value = 5885, .pebbles = 12, .missing_connections = { {1,6}, {3,7}, {1,0}, {2,9}, {9,8}, {2,8}, {5,7}, {11,4}, {10,1}, {5,3}, {0,10}, } },
      { .value = 5818, .pebbles = 12, .missing_connections = { {9,0}, {7,1}, {6,4}, {8,6}, {4,2}, {11,5}, {5,3}, {9,7}, {8,4}, {2,8}, {10,11}, {5,10}, {10,3}, {9,1}, } },
      { .value = 4880, .pebbles = 10, .missing_connections = { {7,3}, {4,1}, {9,2}, {6,9}, {2,6}, {5,0}, {8,4}, } },
      { .value = 5233, .pebbles = 10, .missing_connections = { {0,2}, {4,5}, {8,3}, {9,7}, {7,1}, {6,3}, {6,8}, } },
      { .value = 6208, .pebbles = 12, .missing_connections = { {1,7}, {3,4}, {10,7}, {0,3}, {8,2}, {5,1}, {9,11}, {0,6}, {6,3}, {10,1}, {0,4}, } },
      { .value = 4182, .pebbles = 8, .missing_connections = { {5,7}, {7,4}, {4,5}, {1,0}, {5,6}, {3,1}, {6,4}, {0,3}, {6,7}, } },
    }
  },
  Test{
    .expected_value = 1, .max_time = 100, .transition_time = 8,
    .vaults = {
      { .value = 1, .pebbles = 14, .missing_connections = {
        {0,1}, {0,2}, {3,4}, {4,5}, {6,7}, {8,9}, {10,11}, {12,13},
      } },
    }
  },
};

int main() {
  int ok = 0, fail = 0;

  for (auto t : TESTS) {
    try {
      auto sol = plan_heist(t.vaults, t.transition_time, t.max_time);
      check_solution(sol, t.expected_value, t.vaults, t.transition_time, t.max_time);
      ok++;
    } catch (const TestFailed&) {
      fail++;
    }
  }

  if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
  else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif
