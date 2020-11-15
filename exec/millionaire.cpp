#include <emp-tool/emp-tool.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include "tinygarble/program_interface.h"
#include "tinygarble/program_interface_sh.h"
#include "tinygarble/TinyGarble_config.h"

using namespace std;
namespace po = boost::program_options;

void millionaire(auto TGPI){	

	uint64_t bit_width = 64;
    int64_t a = 0, b = 0;
	
	cout << "Input wealth: ";
    if (TGPI->party == ALICE){
        cin >> a;
        cout << "ALICE's wealth: $" << a << endl;
    }
    else{
        cin >> b;
        cout << "BOB's wealth: $" << b << endl;
    }
    
    auto a_x = TGPI->TG_int_init(ALICE, bit_width, a);
    auto b_x = TGPI->TG_int_init(BOB, bit_width, b);

    TGPI->gen_input_labels();

    TGPI->retrieve_input_labels(a_x, ALICE, bit_width);
    TGPI->retrieve_input_labels(b_x, BOB, bit_width);

    auto res_x = TGPI->TG_int(1);
    TGPI->lt(res_x, a_x, b_x, bit_width);
    int64_t res = TGPI->reveal(res_x, 1, false);

#if !SEC_SH
    if (TGPI->party == BOB) //authenticated garbling allows only evaluator to compute the result
#endif
        if (res == 1) cout << "BOB is richer" << endl;
        else cout << "ALICE is richer" << endl;

	TGPI->clear_TG_int(a_x);
	TGPI->clear_TG_int(b_x);
	TGPI->clear_TG_int(res_x);
	
	delete TGPI;
}

int main(int argc, char** argv) {
	int party = 1, port = 1234;
	string netlist_address;
	string server_ip;
	
	po::options_description desc{"Yao's Millionair's Problem \nAllowed options"};
	desc.add_options()  //
	("help,h", "produce help message")  //
	("party,k", po::value<int>(&party)->default_value(1), "party id: 1 for garbler, 2 for evaluator")  //
	("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
	("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"), "server's IP.")
	("sh", "semi-honest setting (default is malicious)");
	
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
		
	NetIO* io = new NetIO(party==ALICE ? nullptr:server_ip.c_str(), port, true);
	io->set_nodelay();
	
	TinyGarblePI_SH* TGPI_SH;
	TinyGarblePI* TGPI; 
	
	if (vm.count("sh")){
		cout << "testing program interface in semi-honest setting" << endl;
		TGPI_SH = new TinyGarblePI_SH(io, party);
		io->flush();
		millionaire(TGPI_SH);		
	}
	else {
		cout << "Millionair's Problem in malicious setting" << endl;
		TGPI = new TinyGarblePI(io, party, 192, 64);
		io->flush();
		millionaire(TGPI);
	}
	
	delete io;
	
	return 0;
}
