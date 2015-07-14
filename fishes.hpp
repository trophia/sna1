#pragma once

#include <vector>
#include <thread>

#include "dimensions.hpp"

/**
 * A fish
 *
 */
class Fish {
public:

    Time birth;
    Time death;

    Sex sex;

    float length;

    Fish(void){
        //! @randomise initial attribute values
        birth = now;
        death = 0;
        sex = male;
        length = 0;
    }

    Fish& survive(void){
        //! @todo probabalistic mortality with given rate
        //death = now;
        //distribution(random);
        return *this;
    }

    Fish& grow(void){
        //! @todo implement real growth curve with random
        //variation
        auto incr = 10-0.1*length;// + distribution(random);
        length += incr;
        return *this;
    }

    Fish& move(void){
        return *this;
    }

    Fish& update(const Environ& environ){
        survive();
        grow();
        move();
        return *this;
    }

}; // end class Fish

class Fishes : public std::vector<Fish> {
public:

    typedef std::vector<Fish> Base;

    unsigned int start_number;

    void start(const Environ& environ){
        Base::clear();
        Base::resize(start_number);
    }

    #ifndef FISHES_PARALLEL

   void update(const Environ& environ){
        for(Fish& fish : *this){
            fish.update(environ);
        }
    }

    #else

    void update(const Environ& environ){
        int each = Base::size()/6;

        std::thread thread0(update_task,this,environ,0,each);
        std::thread thread1(update_task,this,environ,each,each);
        std::thread thread2(update_task,this,environ,each*2,each);
        std::thread thread3(update_task,this,environ,each*3,each);
        std::thread thread4(update_task,this,environ,each*4,each);
        std::thread thread5(update_task,this,environ,each*5,each);

        thread0.join();
        thread1.join();
        thread2.join();
        thread3.join();
        thread4.join();
        thread5.join();
    }
    static void update_task(Fishes* fishes, const Environ& environ, const unsigned int& start, const unsigned int& num){
        auto fish = (*fishes).begin()+start;
        auto end = fish+num;
        while(fish!=end){
            fish->update(environ);
            fish++;
        }
    }

    #endif
};  // end class Fishes
