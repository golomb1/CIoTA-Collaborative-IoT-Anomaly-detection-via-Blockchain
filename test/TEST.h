//
// Created by golombt on 20/12/2017.
//

#ifndef TEST_TEST_H
#define TEST_TEST_H

#include <iostream>

#define ASSERT(X,Y) {if((X)!=(Y)){std::cout << "Assertion FAILED: got " << (X) << ", expected " << (Y) << std::endl; throw 0; }}


#endif //TEST_TEST_H
