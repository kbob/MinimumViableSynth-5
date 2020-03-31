#ifndef BIJECTION_included
#define BIJECTION_included

#include "noalloc.h"

template <class T, int N>
class bijection {

public:

    void push_back(const T& t)
    {
        m_vec.push_back(t);
    }

    const T& at(size_t i) const
    {
        if (i >= m_vec.size())
            throw std::domain_error("bijection");
        return m_vec.at(i);
    }

    size_t index(const T& t) const
    {
        auto it = std::find(m_vec.begin(), m_vec.end(), t);
        if (it == m_vec.end())
            throw std::domain_error("bijection");
        return it - m_vec.begin();
    }

private:

    fixed_vector<T, N> m_vec;

    friend class BijectionUnitTest;

};

#endif /* !BIJECTION_included */
