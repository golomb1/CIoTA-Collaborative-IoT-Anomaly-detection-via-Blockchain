//
// Created by tomer on 21/01/18.
//

#include <iostream>
#include <random>
#include "../../headers/CppUtils/Utils.h"

void waitForInput(){
    std::cout << '\n' << "To exit press any key to continue...\n";
    std::cin.get();
}


int generateRandomNumber(int min, int max) {
    std::random_device r;

    // Choose a random mean between 1 and 6
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(min, max);
    return uniform_dist(e1);
}