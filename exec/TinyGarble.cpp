#include <emp-tool/emp-tool.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <sys/resource.h>
#include "tinygarble/emp-ag2pc.h"
#include "exec/sequential_execution.h"
#include "exec/sequential_execution_sh.h"
#include "tinygarble/TinyGarble_config.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv) {
	int party = 1, port = 1234;
	string netlist_address = string(NETLIST_PATH_PI) + "add_8bit.emp.bin";
	string server_ip = "127.0.0.1";
	string input_hex_str = "0", init_hex_str = "0", output_hex_str = "0";
	int cycles = 1, repeat = 1, output_mode = 0;	
	bool report = true;
	string in_file;
	int bs_mal = INT_MAX;
	struct rusage usage;
	double memory_usage;
	
	po::options_description desc{"TinyGarble: sequential execution of two party Garbled Circuit protocol \nAllowed options"};
	desc.add_options()  //
	("help,h", "produce help message")  //
	("party,k", po::value<int>(&party)->default_value(1), "party id: 1 for garbler, 2 for evaluator")  //
	("netlist,n", po::value<string>(&netlist_address)->default_value(string(NETLIST_PATH_PI) + "add_8bit.emp.bin"), "circuit file address.")  //
	("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
	("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"), "server's IP.")  //
	("input,i", po::value<string>(&input_hex_str)->default_value("0"),"hexadecimal input (little endian), without init.")  //
	("init,j", po::value<string>(&init_hex_str)->default_value("0"),"hexadecimal init (little endian).") //
	("cycles,c", po::value<int>(&cycles)->default_value(1),"number of cycles to run") //
	("repeat,r", po::value<int>(&repeat)->default_value(1),"number of times to repeat the run") //
	("output_mode,m", po::value<int>(&output_mode)->default_value(0),"0: reveal output at every cycle\n1: reveal output at last cycle\n\
	2: transfer output to next netlist at every cycle\n3: transfer output to next netlist at last cycle") //
	("file,f", po::value<string>(&in_file),"netlist, input, init, cycles, repeat, output_mode read from this file,\n\
	ignores command line inputs for these fields \ncurrently supports only semi-honest setting") //
	("batch_size,b", po::value<int>(&bs_mal)->default_value(INT_MAX),"pre-processing bacth size for malicious setting\n\
	default:choose adaptively\nused for setting maximum available memory") //
	("sh", "sequential execution in semi-honest setting") //
	("oo", "report output only");
	
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
	
	if (vm.count("oo")) report = false;
	
	uint64_t dc[4];
	double dt[4];
		
	NetIO* io = new NetIO(party==ALICE ? nullptr:server_ip.c_str(), port, !report);
	io->set_nodelay();
		
	if (vm.count("sh")){
		if (vm.count("file")){
			if (report) cout << "Concateneted netlist execution in semi-honest settings\n" << endl;
			output_hex_str = sequential_execution_sh(party, io, in_file, repeat, report, dc, dt);
		}
		else {
			if (report) cout << "Single netlist execution in semi-honest settings\n" << endl;
			output_hex_str = sequential_execution_sh(party, io, netlist_address, input_hex_str, init_hex_str, cycles, repeat, output_mode, report, dc, dt);
		}
	}
	else{
		if (vm.count("file")){
			if (report) cout << "Concateneted netlist execution in malicious settings\n" << endl;
			output_hex_str = sequential_execution(party, io, in_file, repeat, bs_mal, report, dc, dt);
		}
		else{
			if (report) cout << "Single netlist execution in malicious setting\n" << endl;
			output_hex_str = sequential_execution(party, io, netlist_address, input_hex_str, init_hex_str, cycles, repeat, output_mode, bs_mal, report, dc, dt);
		}
	}	
	
	getrusage(RUSAGE_SELF, &usage);	
	memory_usage = (double)usage.ru_maxrss/1024;
	if (report) cout << "Memory Usage: " << memory_usage << " MB" << endl;
	
	if (report && (vm.count("sh"))) cout << "Transferred Data: " << (float)(io->num_bytes_sent)/1024/1024 << " MB" << endl;
	
	if (report) cout << "Party " << party << "\nInput\t" << input_hex_str << "\nOutput\t" << output_hex_str << endl;
	else {
		cout << output_hex_str << "\t";	
		uint64_t DC = 0;
		double DT = 0;
		for (int j = 0; j < 4; ++j){
			cout << dc[j] << "\t" << dt[j] << "\t";
			DC += dc[j];
			DT += dt[j];
		}
		cout << DC << "\t" << DT << "\t";
		cout << memory_usage << "\t";
		if (vm.count("sh")) cout << (float)(io->num_bytes_sent)/1024/1024 << endl;
	}	
	
	delete io;
	
	return 0;
}
