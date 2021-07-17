#include <iostream>
#include "Messages.h"

using namespace Messages;

int main()
{
    SDCDMessage m1, m2;
    printf("%d\n", m1.nseq);
    printf("%d\n", m2.nseq);

    std::cout << m1.getMessage() << std::endl;
}
