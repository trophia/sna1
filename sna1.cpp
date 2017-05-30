#include "model.hpp"

int main(int argc, char** argv) {
    Model model;
    model.initialise();

    try {
        std::string task;
        if (argc >= 2) {
            task = argv[1];
        }
        if (task == "run") {
            std::function<void()> callback([&](){
                std::cout
                    << now << "\t"
                    << sum(model.fishes.biomass_spawners) << "\n";
            });
            model.run(1900, 2018, &callback);
        } else {
            std::cout << "No task (e.g. run) specified" <<std::endl;
        }
    } catch(std::exception& error) {
        std::cout << "************Error************\n"
                  << error.what() <<"\n"
                  << "******************************\n";
        return 1;
    } catch(...) {
        std::cout << "************Unknown error************\n";
        return 1;
    }

    model.finalise();

    return 0;
}
