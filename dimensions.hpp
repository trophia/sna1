#pragma once

#include <stencila/dimension.hpp>
using Stencila::Dimension;

/**
 * Time
 */
typedef uint Time;
Time now;

const uint times_per_year = 1;

uint year(Time t) {
    // TODO
    return t;
}

uint quarter(Time t) {
    // TODO
    return 0;
}

float years(Time a, Time b) {
    return (a-b)/times_per_year;
}

/**
 * Region
 */
enum Region {
    EN = 0,
    HG = 1,
    BP = 2
};
STENCILA_DIM(Regions,regions,region,3);

/**
 * Area
 */
typedef uint Area;
STENCILA_DIM(Areas,areas,area,9);
STENCILA_DIM(AreaTos,area_tos,area_to,9);  // For defining and accessing movement matrix


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
 * Harvest methods
 */
enum Method {
    LI = 0,
    TR = 1,
    DS = 2,
    RE = 3
};
STENCILA_DIM(Methods,methods,method,4);
