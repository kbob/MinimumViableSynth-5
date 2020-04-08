#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

template <class Container>
class indexed : public Container {

public:

    size_t index(typename Container::const_reference item) const
    {
        size_t i = 0;
        for (const auto& it: *this) {
            if (it == item)
                return i;
            i++;
        }
        throw std::runtime_error("indexed");
    }
};

int main()
{
    // indexed_vector<float> v;
    indexed<std::vector<float>> v;
    v.push_back(3.3f);
    v.push_back(5.5f);
    v.push_back(7.7f);

    for (const auto& f: v) {
        std::cout << f << " at " << v.index(f) << std::endl;
    }

    try {
        v.index(1.1f);
    } catch (std::runtime_error x) {
        std::cerr << "caught runtime error: " << x.what() << std::endl;
    }

    return 0;
}
