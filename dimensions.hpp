#pragma once

/**
 * Time
 */
typedef uint Time;
Time now;

const uint times_per_year = 1;

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

/**
 * Stock of fish
 */
enum Stock {
    W = 0,
    E = 1
};

/**
 * Sex of fish
 */
enum Sex {
    male = 1,
    female = 2
};

/**
 * Fishing methods
 */
enum {
    TR = 0,
    LI = 1
};

