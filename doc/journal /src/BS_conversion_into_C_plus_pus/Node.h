#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <algorithm>

class Node {
public:
    // Indeksi zadnjeg meča za svaku sekvencu (-1 = prije početka)
    std::vector<int> pos;

    // Trenutno izgrađeni LCS (ako se ne rekonstruiše preko parent-a)
    std::string seq;

    // Pokazivač na roditeljski čvor
    Node* parent;

    // Konstruktor
    Node(const std::vector<int>& pos_,
         const std::string& seq_,
         Node* parent_ = nullptr)
        : pos(pos_), seq(seq_), parent(parent_) {}

    // Ekvivalent @property length
    int length() const {
        return static_cast<int>(seq.size());
    }

    // ============================================================
    // Generisanje nasljednika (ekvivalent generate_successors)
    // ============================================================
    static std::vector<Node*> generateSuccessors(
        Node* node,
        const std::vector<std::string>& sequences,
        const std::vector<std::vector<int>>& gaps
    ) {
        const int m = sequences.size();

        // per_seq_maps[i] : map<char, index>
        std::vector<std::unordered_map<char, int>> perSeqMaps(m);

        for (int i = 0; i < m; ++i) {
            perSeqMaps[i] = firstValidIndices(
                sequences[i],
                node->pos[i] + 1,
                gaps[i],
                node->pos[i]
            );
        }

        // --------------------------------------------------------
        // Presjek karaktera koje sve sekvence mogu mečovati
        // --------------------------------------------------------
        std::set<char> commonChars;
        bool first = true;

        for (const auto& mp : perSeqMaps) {
            std::set<char> chars;
            for (const auto& kv : mp) chars.insert(kv.first);

            if (first) {
                commonChars = chars;
                first = false;
            } else {
                std::set<char> tmp;
                std::set_intersection(
                    commonChars.begin(), commonChars.end(),
                    chars.begin(), chars.end(),
                    std::inserter(tmp, tmp.begin())
                );
                commonChars.swap(tmp);
            }
        }

        if (commonChars.empty())
            return {};

        // --------------------------------------------------------
        // Uklanjanje dominirajućih simbola
        // --------------------------------------------------------
        std::set<char> dominated;

        for (char c : commonChars) {
            for (char o : commonChars) {
                if (c == o) continue;

                bool dominates = true;
                for (int i = 0; i < m; ++i) {
                    int dc = perSeqMaps[i].at(c);
                    int do_ = perSeqMaps[i].at(o);
                    if (!(dc <= do_ &&
                          gaps[i][do_] + dc + 1 >= do_)) {
                        dominates = false;
                        break;
                    }
                }
                if (dominates)
                    dominated.insert(o);
            }
        }

        for (char d : dominated)
            commonChars.erase(d);

        // --------------------------------------------------------
        // Kreiranje novih Node-ova
        // --------------------------------------------------------
        std::vector<Node*> successors;

        for (char ch : commonChars) {
            std::vector<int> idxVector(m);

            for (int i = 0; i < m; ++i)
                idxVector[i] = perSeqMaps[i].at(ch);

            Node* child = new Node(
                idxVector,
                node->seq + ch,
                node
            );

            successors.push_back(child);
        }

        return successors;
    }

private:
    // ============================================================
    // Helper: _first_valid_indices
    // ============================================================
    static std::unordered_map<char, int> firstValidIndices(
        const std::string& seq,
        int startIdx,
        const std::vector<int>& gapArr,
        int prevIdx
    ) {
        std::unordered_map<char, int> mapping;

        for (int j = startIdx; j < static_cast<int>(seq.size()); ++j) {
            if (prevIdx != -1 && (j - prevIdx) > gapArr[j] + 1)
                continue;

            char c = seq[j];
            if (mapping.find(c) == mapping.end()) {
                mapping[c] = j; // prvi (najlijevlji) hit
            }
        }
        return mapping;
    }
};

#endif // NODE_H

