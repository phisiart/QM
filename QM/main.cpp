#include <bitset>
#include <iostream>
#include <algorithm>
#include <vector>

const int MAX_SIZE = 10;
size_t length = 10;

//typedef std::bitset<MAX_SIZE> Atom;

class Atom : public std::bitset<MAX_SIZE> {
public:
    Atom() : std::bitset<MAX_SIZE>(), flag(0) {}
    Atom(int input) : std::bitset<MAX_SIZE>(input), flag(0) {}
    Atom(const std::bitset<MAX_SIZE>& input) : std::bitset<MAX_SIZE>(input), flag(0) {}

    std::bitset<MAX_SIZE> barbits;

    bool operator==(const Atom& rhs) const {
        return (std::bitset<MAX_SIZE>::operator==(rhs)) && (barbits == rhs.barbits);
    }

    bool Cover(const std::bitset<MAX_SIZE>& minterm) {
        std::bitset<MAX_SIZE> compare = std::bitset<MAX_SIZE>(*this) ^ minterm;
        return ((compare | barbits) == barbits);
    }

    // flag:
    // -1: invalid: something wrong.
    //  0: prime.
    //  1: merged.
    int flag;
    //size_t length;
    
};

std::ostream& operator<<(std::ostream& os, const Atom& atom) {
    for (int i = length - 1; i >= 0; --i) {
        os << (atom.barbits.at(i) ? '-' : (atom.at(i) ? '1' : '0'));
    }
    os << ' ' << (atom.flag ? 'm' : '*');
    return os;
}


// Two atoms can merge iff:
// 1. Their barbits are the same.
// 2. They only differs in one position.
Atom Merge(const Atom& atom1, const Atom& atom2) {
    Atom ret(atom1 | atom2);
    if ((atom1.barbits != atom2.barbits) || ((atom1 ^ atom2).count() != 1)) {
        ret.flag = -1;
        return ret;
    }

    // Get the new barbits
    ret.barbits = atom1.barbits | (atom1 ^ atom2);
    return ret;
}

class Group {
public:
    std::vector<Atom> atoms;
};

class Column {
public:
    std::vector<Group> groups;
};

std::ostream& operator<<(std::ostream& os, const Column& column) {

    os << "Column:" << std::endl;
    for (size_t i = 0; i < column.groups.size(); ++i) {
        for (size_t j = 0; j < column.groups[i].atoms.size(); ++j) {
            os << column.groups[i].atoms[j] << std::endl;
        }
        os << "----------" << std::endl;
    }
    os << std::endl;
    return os;
}


class MatchTable {
public:
    std::vector<Atom> primes;
    std::vector<Atom> minterms;
    std::vector<std::vector<bool> > checked;

    void ConstructCheckList() {
        checked.resize(minterms.size());
        for (size_t i = 0; i < checked.size(); ++i) {
            checked[i].resize(primes.size());
            // todo...
            for (size_t j = 0; j < primes.size(); ++j) {
                if (primes[j].Cover(minterms[i])) {
                    checked[i][j] = true;
                } else {
                    checked[i][j] = false;
                }
            }
        }
    }

    std::vector<Atom> GetAnswer() {
        std::vector<Atom> ret;
        if (minterms.empty()) {
            return ret;
        }

        // Step 1. Find those minterms that are only covered by a single prime.
        for (size_t i = 0; i < minterms.size(); ++i) {
            if (std::count(checked[i].cbegin(), checked[i].cend(), true) == 1) {
                size_t minterm_found = i;
                size_t prime_found = std::find(checked[i].cbegin(), checked[i].cend(), true) - checked[i].cbegin();

                ret.push_back(primes[prime_found]);
                
                MatchTable next;
                for (size_t j = 0; j < minterms.size(); ++j) {
                    if (!checked[j][prime_found]) {
                        next.minterms.push_back(minterms[j]);
                    }
                }
                for (size_t j = 0; j < primes.size(); ++j) {
                    if (j != prime_found) {
                        next.primes.push_back(primes[j]);
                    }
                }
                next.ConstructCheckList();
                std::vector<Atom> append(next.GetAnswer());
                ret.insert(ret.cend(), append.begin(), append.end());
                return ret;
            }
        }

        // Step 2. Choose any minterm.
        std::vector<std::vector<Atom> > rets;
        rets.resize(primes.size());
        size_t min = 0;
        for (size_t i = 0; i < primes.size(); ++i) {
            size_t prime_found = i;
            rets[i].push_back(primes[prime_found]);
            MatchTable next;
            for (size_t j = 0; j < minterms.size(); ++j) {
                if (!checked[j][prime_found]) {
                    next.minterms.push_back(minterms[j]);
                }
            }
            for (size_t j = 0; j < primes.size(); ++j) {
                if (j != prime_found) {
                    next.primes.push_back(primes[j]);
                }
            }
            next.ConstructCheckList();
            std::vector<Atom> append(next.GetAnswer());
            rets[i].insert(rets[i].cend(), append.begin(), append.end());
            if (rets[i].size() < rets[min].size()) {
                min = i;
            }
        }

        return rets[min];

    }

    void Print() {
        std::cout << "Primes:" << std::endl;
        for (auto& prime : primes) {
            std::cout << prime << std::endl;
        }
        std::cout << "Minterms:" << std::endl;
        for (auto& minterm : minterms) {
            std::cout << minterm << std::endl;
        }

        for (size_t i = 0; i < checked.size(); ++i) {
            for (size_t j = 0; j < primes.size(); ++j) {
                std::cout << (checked[i][j] ? 'c' : ' ');
            }
            std::cout << std::endl;
        }
    }

};


class QM {
public:
    QM(int num_of_vars,
       std::vector<int>& minterms,
       std::vector<int>& dontcares) : num_of_vars(num_of_vars) {

        columns.resize(num_of_vars + 1);

        for (size_t i = 0; i < columns.size(); ++i) {
            // columns[i].groups.resize(columns.size() - i); // should be better.
            columns[i].groups.resize(columns.size());

        }

        // Load the first column.
        for (size_t i = 0; i < minterms.size(); ++i) {
            Atom atom(minterms[i]);
            int count = atom.count();
            columns[0].groups[count].atoms.push_back(atom);
            min_terms.push_back(atom);
        }

        for (size_t i = 0; i < dontcares.size(); ++i) {
            Atom atom(dontcares[i]);
            int count = atom.count();
            columns[0].groups[count].atoms.push_back(atom);
            dont_cares.push_back(atom);
        }

        for (size_t i = 0; i < length; ++i) {
            ConstructColumn(i);
        }

        std::cout << columns[0] << columns[1] << columns[2];

        MatchTable match_table;
        for (auto& minterm : min_terms) {
            match_table.minterms.push_back(minterm);
        }
        for (auto& column : columns) {
            for (auto& group : column.groups) {
                for (auto& atom : group.atoms) {
                    if (atom.flag == 0) {
                        match_table.primes.push_back(atom);
                    }
                }
            }
        }
        match_table.ConstructCheckList();
        match_table.Print();

        auto answer = match_table.GetAnswer();
        std::cout << "Answer:" << std::endl;
        for (auto& atom : answer) {
            std::cout << atom << std::endl;
        }
        std::cout << std::endl;
    }

    void ConstructColumn(size_t from) {
        for (int i = 0; i < columns[from + 1].groups.size(); ++i) {
            columns[from + 1].groups[i].atoms.clear();
        }

        for (size_t groupidx = 0; groupidx < columns[from].groups.size() - 1; ++groupidx) {
            for (size_t i = 0; i < columns[from].groups[groupidx].atoms.size(); ++i) {
                for (size_t j = 0; j < columns[from].groups[groupidx + 1].atoms.size(); ++j) {
                    Atom atom = Merge(columns[from].groups[groupidx].atoms[i], columns[from].groups[groupidx + 1].atoms[j]);
                    if (atom.flag != -1) {
                        columns[from].groups[groupidx].atoms[i].flag = columns[from].groups[groupidx + 1].atoms[j].flag = 1;
                        if (std::find(
                                columns[from + 1].groups[groupidx].atoms.cbegin(),
                                columns[from + 1].groups[groupidx].atoms.cend(),
                                atom
                            ) == columns[from + 1].groups[groupidx].atoms.cend()) {
                            columns[from + 1].groups[groupidx].atoms.push_back(atom);
                        }
                        
                    }
                }
            }
        }

    }

    int num_of_vars;
    std::vector<Column> columns;
    std::vector<Atom> min_terms;
    std::vector<Atom> dont_cares;
    
};


int main() {

    size_t num_of_vars;// = 5;
    size_t num_of_minterms;// = 7;
    size_t num_of_dontcares;// = 3;

    std::vector<int> minterms;
    std::vector<int> dontcares;

    std::cout << "Number of variables = ";
    std::cin >> num_of_vars;
    length = num_of_vars;

    std::cout << "Number of minterms = ";
    std::cin >> num_of_minterms;
    
    std::cout << "Input minterms: ";
    for (size_t i = 0; i < num_of_minterms; ++i) {
        int minterm;
        std::cin >> minterm;
        minterms.push_back(minterm);
    }
    
    std::cout << "Number of dont't cares = ";
    std::cin >> num_of_dontcares;

    for (size_t i = 0; i < num_of_dontcares; ++i) {
        int dontcare;
        std::cin >> dontcare;
        dontcares.push_back(dontcare);
    }

    //std::vector<int> minterms = { 4, 5, 6, 8, 9, 10, 13 };
    //std::vector<int> dontcares = { 0, 7, 15 };

    //std::vector<int> minterms = { 2, 3, 7, 10, 12, 15, 27 };
    //std::vector<int> dontcares = { 5, 18, 19, 21, 23 };
    QM qm(num_of_vars, minterms, dontcares);

    return 0;
}
