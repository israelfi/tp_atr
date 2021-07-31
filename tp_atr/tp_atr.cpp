#include <iostream>
#include "Messages.h"

using namespace Messages;

int main()
{
    PIMSMessage m1(2);
    //printf("%d\n", m1.nseq);
    std::cout << m1.getMessage() << std::endl;
    getchar();

    PIMSMessage m2(9);
    std::cout << m2.getMessage() << std::endl;
    getchar();

    PIMSMessage m3(2);
    std::cout << m3.getMessage() << std::endl;
    getchar();

    PIMSMessage m4(9);
    std::cout << m4.getMessage() << std::endl;
    getchar();

    PIMSMessage m5(2);
    std::cout << m5.getMessage() << std::endl;
    getchar();

    PIMSMessage m6(9);
    std::cout << m6.getMessage() << std::endl;
}
