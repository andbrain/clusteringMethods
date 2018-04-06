#pragma once
#include <vector>
#include <iostream>

#include <ctime>
#include <sys/time.h>

#include "mat.h"

std::vector<int> affinityPropagation(
    mat* S,
    int prefType = 1,
    double damping = 0.9,
    int maxit = 1000,
    int convit = 50
);
