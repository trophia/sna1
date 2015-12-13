#pragma once

#include "environ.hpp"
#include "fishes.hpp"
#include "fleet.hpp"

/**
 * The model
 *
 * Links together the sub-models `Environ`, 'Fishes` and `Fleet`
 */
class Model {
public:

    Environ environ;
    Fishes fishes;
    Fleet fleet;

    Model& initialise(void){
        environ.initialise();
        fishes.initialise();
        fleet.initialise();

        return *this;
    }

    Model& update(void){
        fishes.track();

        environ.update();
        fishes.update(environ);
        //fleet.update(fishes,environ);

        return *this;
    }

    Model& run(unsigned int from, unsigned int to){
        now = from;
        start();
        while(now<=to){
            std::cout<<now<<"\t"<<fishes.fishes.size()<<std::endl;
            update();
            now++;
        }
        stop();
        
        return *this;
    }
}; // end class Model
