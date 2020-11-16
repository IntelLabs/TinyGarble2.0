#include <emp-tool/emp-tool.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include "tinygarble/TinyGarble_config.h"
#include "tinygarble/program_interface.h"
#include "tinygarble/program_interface_sh.h"
#include "exec/exec_common.h"
#include "exec/unit_test.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv) {
	int party = 1, port = 1234;
	string netlist_address;
	string server_ip;
	int program;
	int bs_mal = 100000;
	
	Timer T;
	memMeter M;
	T.start();
	
	po::options_description desc{"Unit test for TinyGarble programming interface \nAllowed options"};
	desc.add_options()  //
	("help,h", "produce help message")  //
	("party,k", po::value<int>(&party)->default_value(1), "party id: 1 for garbler, 2 for evaluator")  //
	("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
	("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"), "server's IP.")  //
	("batch_size,b", po::value<int>(&bs_mal)->default_value(100000),"pre-processing bacth size for malicious setting\n\
	default:choose adaptively\nused for setting maximum available memory") //
	("sh", "semi-honest setting (default is malicious)") ;
	
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

	srand(time(NULL));
		
	NetIO* io = new NetIO(party==ALICE ? nullptr:server_ip.c_str(), port, true);
	io->set_nodelay();
	
	TinyGarblePI_SH* TGPI_SH;
	TinyGarblePI* TGPI; 
	
	if (vm.count("sh")){
		cout << "testing program interface in semi-honest setting" << endl;
		TGPI_SH = new TinyGarblePI_SH(io, party);
		io->flush();
		unit_test(TGPI_SH);		
	}
	else {
		cout << "testing program interface in malicious setting with batch size " << bs_mal << endl;
		TGPI = new TinyGarblePI(io, party, bs_mal, bs_mal);
		io->flush();
		unit_test(TGPI);
	}
	
	delete io;
	
	T.print("end");
	M.print("end");
	
	return 0;
}
