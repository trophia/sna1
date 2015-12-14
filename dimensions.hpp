#pragma once

#include <stencila/dimension.hpp>
using Stencila::Dimension;

/**
 * Time
 */
typedef uint Time;
Time now;

const uint times_per_year = 1;

uint year(Time a) {
    return a;
}

float years(Time a, Time b) {
    return (a-b)/times_per_year;
}

/**
 * Area
 */
enum Area {
    EN = 0,
    HG = 1,
    BP = 2
};
STENCILA_DIM(Areas,areas,area,3);
STENCILA_DIM(AreaTos,area_tos,area_to,3);  // For defining and accessing movement matrix

/**
 * Stock of fish
 */
enum Stock {
    W = 0,
    E = 1
};
STENCILA_DIM(Stocks,stocks,stock,2);

/**
 * Sex of fish
 */
enum Sex {
    male = 0,
    female = 1
};
STENCILA_DIM(Sexes,sexes,sex,2);

/**
 * Age of fish
 */
STENCILA_DIM(Ages,ages,age,31);

int age_bin(double age){
    return std::min(age,30.0);
}

/**
 * Length of fish
 */
STENCILA_DIM(Lengths,lengths,length,51);

int length_bin(double length){
    return std::min(length,100.0)/2;
}

/**
 * Fishing methods
 */
enum {
    TR = 0,
    LI = 1
};
STENCILA_DIM(Methods,methods,method,2);
