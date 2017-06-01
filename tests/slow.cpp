#define BOOST_TEST_MODULE tests_slow
#include <boost/test/unit_test.hpp>

/**
 * Slower integration tests that are run less frequently than 
 * unit tests (because they are slow)
 *
 * These tests are run by changing into the test directory, running
 * the model and then doing something with it's output files e.g.
 * running CASAL, running tagging experiment code
 */

#include "../model.hpp"

BOOST_AUTO_TEST_SUITE(slow)

BOOST_AUTO_TEST_CASE(run){

    auto home = boost::filesystem::current_path();

    for (auto folder : std::vector<std::string>({
        "tests/tagging/simple",
        "tests/casal/length-default",
        "tests/casal/run-x"
    })) {
        boost::filesystem::current_path(folder);

        // Run the model
        std::cout << "\n" << folder << std::endl;
        try {
            Model model;
            model.initialise();
            std::function<void()> callback([&](){
                std::cout << "." << std::flush;
            });
            model.run(1900, 2018, &callback);
            model.finalise();
        } catch(std::exception& error) {
            BOOST_FAIL(error.what());
        } catch(...) {
            BOOST_FAIL("Exception");
        }

        // If there is a test.sh file then run that
        if (boost::filesystem::exists("test.sh")) {
            auto ok = std::system("bash test.sh");
            BOOST_CHECK(ok==0);
        }

        boost::filesystem::current_path(home);
    }

}

BOOST_AUTO_TEST_SUITE_END()
