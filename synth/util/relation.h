#ifndef RELATION_included
#define RELATION_included

#include "synth/util/universe.h"

template <class T1, class T2, size_t N1, size_t N2>
class Relation {

public:
    Relation(const Universe<T1, N1>&u1, const Universe<T2, N2>& u2)
    : m_u1{u1}, m_u2{u2}
    {
        for (size_t i = 0; i < m_u1.size(); i++)
            m_matrix.push_back(m_u2.none);

        // std::cout << "\n\n";
        // for (size_t i = 0; i < m_u1.size(); i++)
        //     std::cout << m_u1[i] << ": " << m_matrix[i] << "\n";
        // std::cout << std::endl;
    }

    bool contains(const T1& v1, const T2& v2) const
    {
        return m_matrix[m_u1.index(v1)].test(m_u2.index(v2));
    }

    const Subset<T2, N2>& at(size_t index) const
    {
        return m_matrix.at(index);
    }

    const Subset<T2, N2>& get(const T1& v1) const
    {
        return m_matrix[m_u1.index(v1)];
    }

    void add(const T1& v1, const T2& v2)
    {
        m_matrix[m_u1.index(v1)].add(v2);
    }

private:
    const Universe<T1, N1>& m_u1;
    const Universe<T2, N2>& m_u2;
    fixed_vector<Subset<T2, N2>, N1> m_matrix;
};

#endif /* !RELATION_included */
