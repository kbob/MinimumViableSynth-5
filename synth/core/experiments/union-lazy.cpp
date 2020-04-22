#include <iostream>

template <class T>
class U {

public:

    U(const T& x)
    {
        std::cout << "U constructor: " << x << std::endl;
        data[0] = x;
    }

   ~U()
    {
        std::cout << "U destructor: " << data[0] << std::endl;
    }

   T data[100];

};

class C {

public:

    C()
    : flag{false}
    {
        std::cout << "C constructor" << std::endl;
    }

    ~C()
    {
        if (flag)
            u_holder.member.~U();
        std::cout << "C destructor" << std::endl;
   }

   void finalize()
   {
       new (&u_holder.member) U<char>('a');
       flag = true;
   }

private:

    template <class T>
    union holder {
        holder() {}
        ~holder() {}
        U<T> member;
    };

    bool flag;
    holder<char> u_holder;

};

int main()
{
    C c;
    c.finalize();
    return 0;
}
