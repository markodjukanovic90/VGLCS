#ifndef INSTANCE_H
#define INSTANCE_H

#include <vector>
#include <string>

class Instance {


public:

    // Ulazne sekvence
    std::vector<std::string> sequences;

    // Gap nizovi
    std::vector<std::vector<int>> gaps;
    
    // Prev
    std::vector<std::vector<<std::vector, int>>> Prev;
    
    // Succ: 
    std::vector<std::vector<<std::vector, int>>> Succ;
    
    // Abeceda Σ (jedinstveni simboli)
    std::vector<char> Sigma;
    
    // Mapiranje znak → indeks (0..|Σ|-1)
    std::unordered_map<char, int> charToInt;

    // C[i][a][j] = frekvencija 'a' u sequences[i][j:]
    std::vector<std::unordered_map<char, std::vector<int>>> C_suffix;

    // Ulazne sekvence
    std::vector<std::string> sequences;

    // Gap nizovi (isti indeks kao sequences)
    std::vector<std::vector<int>> gaps;
    // P structure 
    std::vector<std::vector<double>> P;

    // Konstruktor
    Instance() = default;

    // Učitavanje instance iz fajla
    static Instance loadFromFile(const std::string& filepath);

    // Broj sekvenci
    int numSequences() const {
        return static_cast<int>(sequences.size());
    }

    // Dužina i-te sekvence
    int sequenceLength(int i) const {
        return static_cast<int>(sequences[i].size());
    }
    
    // Izgradnja Σ i C
    void buildAlphabet();
    void buildSuffixCounts();
    void buildCharToInt();
    // neede for backward BS
    void buildPrevTable();
    // needed for forwards BS 
    void buildSuccTable();
    //probability-based heuristic    
    void buildPTable(int max_n);
    int numSequences() const { return sequences.size(); }
};

#endif // INSTANCE_H
