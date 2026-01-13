#include "Instance.h"
#include <fstream>
#include <stdexcept>
#include <unordered_set>

Instance Instance::loadFromFile(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in)
        throw std::runtime_error("Cannot open input file");

    Instance inst;

    int N;
    in >> N;
    if (N < 2)
        throw std::runtime_error("N must be >= 2");

    for (int i = 0; i < N; ++i) {
        std::string seq;
        in >> seq;
        inst.sequences.push_back(seq);

        std::vector<int> g(seq.size());
        for (int j = 0; j < seq.size(); ++j)
            in >> g[j];

        inst.gaps.push_back(g);
    }

    inst.buildAlphabet();
    inst.buildSuffixCounts();

    return inst;
}

void Instance::buildAlphabet() {
    std::unordered_set<char> seen;
    for (const auto& s : sequences)
        for (char c : s)
            seen.insert(c);

    Sigma.assign(seen.begin(), seen.end());
}

void Instance::buildCharToInt() {
    charToInt.clear();
    for (int i = 0; i < Sigma.size(); ++i) {
        charToInt[Sigma[i]] = i;
    }
}

void Instance::buildSuffixCounts() {
    C_suffix.clear();
    C_suffix.reserve(sequences.size());

    for (const std::string& s : sequences) {
        int n = s.size();

        std::unordered_map<char, std::vector<int>> Ci;

        // inicijalizacija: Ci[a] = vector<int>(n+1, 0)
        for (char a : Sigma)
            Ci[a] = std::vector<int>(n + 1, 0);

        // popunjavanje od kraja ka početku
        for (int j = n - 1; j >= 0; --j) {
            char ch = s[j];

            for (char a : Sigma)
                Ci[a][j] = Ci[a][j + 1];

            Ci[ch][j] += 1;
        }

        C_suffix.push_back(std::move(Ci));
    }
}

void Instance::buildPrevTable() {
    int S = sequences.size();
    int K = Sigma.size();

    Prev.clear();
    Prev.resize(S);

    for (int i = 0; i < S; ++i) {
        const std::string& s = sequences[i];
        int n = s.size();

        // Prev[i][c][j]
        Prev[i].assign(K, std::vector<int>(n + 1, -1));

        // last[c] = posljednja pozicija znaka c
        std::vector<int> last(K, -1);

        for (int j = 0; j <= n; ++j) {
            // kopiraj trenutne "last" vrijednosti
            for (int c = 0; c < K; ++c)
                Prev[i][c][j] = last[c];

            if (j < n) {
                int cidx = charToInt[s[j]];
                last[cidx] = j;
            }
        }
    }
}


