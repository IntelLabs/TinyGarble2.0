#include <emp-tool/emp-tool.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <fstream>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv) {
	string netlist_address_in, netlist_address_out;	
	
	po::options_description desc{"Read EMP circuit and write to binary file. \nAllowed options"};
	desc.add_options()  //
	("help,h", "produce help message")  //
	("input,i", po::value<string>(&netlist_address_in), "input text file address.");
	
	po::variables_map vm;
	try {
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
		po::store(parsed, vm);
		if (vm.count("help")) {
			cout << desc << endl;
			return 0;
		}
		po::notify(vm);
	}catch (po::error& e) {
		cout << "ERROR: " << e.what() << endl << endl;
		cout << desc << endl;
		return -1;
	}
	
	netlist_address_out = netlist_address_in + ".bin";
	
	Timer T;
	uint64_t dc_t, dc_b;
	double dt_t, dt_b;
	
	T.start();
	
	CircuitFile cf (netlist_address_in.c_str());
	
	T.get(dc_t, dt_t);	
	
	cf.toBinary(netlist_address_out.c_str());	
	
	T.start();
	
	CircuitFile cf1 (netlist_address_out.c_str(), true);	
	
	T.get(dc_b, dt_b);	
	
	cout << "text file: " << netlist_address_in  << " >> binary file: " << netlist_address_out << "| speed-up: " << dt_t/dt_b << endl;
	
	return 0;
}
