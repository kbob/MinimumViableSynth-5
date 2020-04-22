#ifndef RELATION_included
#define RELATION_included

#include "synth/util/noalloc.h"
#include "synth/util/universe.h"

template <class U1, class U2>
class Relation {

public:

    typedef typename U1::member_type mem1_type;
    typedef typename U2::member_type mem2_type;
    typedef typename U1::subset_type sub1_type;
    typedef typename U2::subset_type sub2_type;
    static const size_t max_size1 = U1::max_size;
    static const size_t max_size2 = U2::max_size;

    Relation(const U1& u1, const U2& u2)
    : m_u1{u1}, m_u2{u2}
    {
        for (size_t i = 0; i < m_u1.size(); i++)
            m_matrix.push_back(m_u2.none);

        // std::cout << "\n\n";
        // for (size_t i = 0; i < m_u1.size(); i++)
        //     std::cout << m_u1[i] << ": " << m_matrix[i] << "\n";
        // std::cout << std::endl;
    }

    bool contains(const mem1_type& v1, const mem2_type& v2) const
    {
        return m_matrix[m_u1.index(v1)].test(m_u2.index(v2));
    }

    const sub2_type& at(size_t index) const
    {
        return m_matrix.at(index);
    }

    const sub2_type& get(const mem1_type& v1) const
    {
        return m_matrix[m_u1.index(v1)];
    }

    void add(const mem1_type& v1, const mem2_type& v2)
    {
        m_matrix[m_u1.index(v1)].add(v2);
    }

private:
    const U1& m_u1;
    const U2& m_u2;
    fixed_vector<sub2_type, max_size1> m_matrix;
};

#endif /* !RELATION_included */
