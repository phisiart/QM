#include <bitset>
#include <iostream>
#include <algorithm>
#include <vector>

const int MAX_SIZE = 10;
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

    // flag:
    // -1: invalid: something wrong.
    //  0: prime.
    //  1: merged.
    int flag;
};

std::ostream& operator<<(std::ostream& os, const Atom& atom) {
    for (int i = MAX_SIZE - 1; i >= 0; --i) {
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
        }

        for (size_t i = 0; i < dontcares.size(); ++i) {
            Atom atom(dontcares[i]);
            int count = atom.count();
            columns[0].groups[count].atoms.push_back(atom);
        }

        //std::cout << columns[0];

        //std::cout << (columns[0].groups[1].atoms[0] | columns[0].groups[1].atoms[1]) << std::endl;
        //std::cout << Merge(columns[0].groups[0].atoms[0], columns[0].groups[1].atoms[0]) << std::endl;

        ConstructColumn(0);
        ConstructColumn(1);

        std::cout << columns[0] << columns[1] << columns[2];
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

    
};



int main() {
    int num_of_vars = 4;
    int num_of_minterms = 7;
    int num_of_dontcares = 3;

    std::vector<int> minterms = { 4, 5, 6, 8, 9, 10, 13 };
    std::vector<int> dontcares = { 0, 7, 15 };
    QM qm(num_of_vars, minterms, dontcares);

    return 0;
}