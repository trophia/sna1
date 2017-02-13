#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "../model.hpp"


BOOST_AUTO_TEST_SUITE(model)


BOOST_AUTO_TEST_CASE(tagging){
	Model model;
	model.initialise();

	// Set up movement
	parameters.fishes_movement_type = 'h';
	parameters.fishes_movement = {
		0.8, 0.1, 0.1,
		0.1, 0.7, 0.2,
		0.1, 0.3, 0.6
	};
	
	// Set up tagging program
	auto& tagging = model.monitor.tagging;

	tagging.releases = 0;
	tagging.releases(2000, EN, LL) =  7500;
	tagging.releases(2000, HG, LL) = 15000;
	tagging.releases(2000, BP, LL) =  7500;

	tagging.release_years = false;
	tagging.release_years(2000) = true;

	tagging.recovery_years = false;
	tagging.recovery_years(2000) = true;
	tagging.recovery_years(2001) = true;
	tagging.recovery_years(2002) = true;
	tagging.recovery_years(2003) = true;

	// Run over a few years
    // Callback function that is called each year
	std::ofstream length_file("output/monitor/length.tsv");
    std::function<void()> callback([&](){
        auto y = year(now);
        if (y>=1900) {
            for (auto region : regions) {
                for (auto method : methods) {
                	for (auto length : lengths) {
                        length_file 
                        	<< y << "\t" 
                        	<< region_code(region) << "\t" 
                        	<< method_code(method) << "\t"
                        	<< length << "\t"
                        	<< model.monitor.length_sample(region, method, length) << "\n";
	                }
                }
            }
        }
    });
	model.run(2000, 2004, &callback);

	// Do checks
	BOOST_CHECK(tagging.number > 0);
	BOOST_CHECK(tagging.tags.size() > 0);

	// Output files for checking
	model.finalise();
}


BOOST_AUTO_TEST_CASE(casal){
	Model model;
	model.initialise();

	// Generate files for CASAL
	model.generate_casal(1900, 2020);

	// Run CASAL
	auto ok = std::system("Rscript tests/casal-files/len.runner.R");
	BOOST_CHECK(ok==0);

	// Read in output files containing CASAL estimates
	std::ifstream file("tests/casal-estimates.txt");
	std::map<std::string, double> estimates;
	std::string variable;
	std::string year;
	std::string stock;
	double estimate;
	std::string header;
	std::getline(file, header);
	while (file >> variable >> year >> stock >> estimate) {
		estimates[variable + "-" + year + "-" + stock] = estimate;
	}

	// Check estimates of B0 by region are within 5%
	BOOST_CHECK_CLOSE(estimates["B0-NA-ENLD"], parameters.fishes_b0(EN), 20);
	BOOST_CHECK_CLOSE(estimates["B0-NA-HAGU"], parameters.fishes_b0(HG), 20);
	BOOST_CHECK_CLOSE(estimates["B0-NA-BOP"], parameters.fishes_b0(BP), 20);

	// Check estimates of R0 by region are within 5%
	BOOST_CHECK_CLOSE(estimates["R0-NA-ENLD"], model.fishes.recruitment_pristine(EN), 5);
	BOOST_CHECK_CLOSE(estimates["R0-NA-HAGU"], model.fishes.recruitment_pristine(HG), 5);
	BOOST_CHECK_CLOSE(estimates["R0-NA-BOP"], model.fishes.recruitment_pristine(BP), 5);
}

BOOST_AUTO_TEST_SUITE_END()
