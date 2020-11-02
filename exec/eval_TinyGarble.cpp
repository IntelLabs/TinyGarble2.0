#include <emp-tool/emp-tool.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include "tinygarble/TinyGarble_config.h"

using namespace std;
namespace po = boost::program_options;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main(int argc, char** argv) {
		
	pid_t pid = fork(); //parent: Alice. child: Bob
	
	int port;
	string netlist_address;
	string server_ip;
	string input_hex_str, init_hex_str, output_hex_str;
	int cycles, repeat, output_mode;
	string in_file, in_file_A, in_file_B;
	int bs_mal = INT_MAX;
	double memory_usage, MEMORY_USAGE;
	double transferred_data, TRANSFERRED_DATA;
	string bench_file, eval_file;	
	string benchmark, bit_width;
	int num_eval;
	
	po::options_description desc{"Evaluation of TinyGarble sequential execution \nAllowed options"};
	desc.add_options()  //
	("help,h", "produce help message")  //
	("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
	("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"), "server's IP.")  //
	("input,i", po::value<string>(&input_hex_str)->default_value("0"),"hexadecimal input (little endian), without init.")  //
	("init,j", po::value<string>(&init_hex_str)->default_value("0"),"hexadecimal init (little endian).") //
	("repeat,r", po::value<int>(&repeat)->default_value(1),"number of times to repeat the run") //
	("file,f", po::value<string>(&bench_file)->default_value(string(BIN_PATH) + "benchmarks.txt"),"netlist, cycles, output_mode read from this file") //
	("batch_size,b", po::value<int>(&bs_mal)->default_value(INT_MAX),"pre-processing bacth size for malicious setting\n\
	default:choose adaptively\nused for setting maximum available memory")  //
	("num_eval,t", po::value<int>(&num_eval)->default_value(10),"number of times to average evaluation") //
	("sh", "semi-honest security model"); 
	
	po::variables_map vm;
	try {
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
		po::store(parsed, vm);
		if (vm.count("help")) {
			if(pid > 0) cout << desc << endl;
			return 0;
		}
		po::notify(vm);
	}catch (po::error& e) {
		cout << "ERROR: " << e.what() << endl << endl;
		cout << desc << endl;
		return -1;
	}	
		
	ifstream fin(bench_file.c_str(), std::ios::in);
	if (!fin.good()){
		perror(bench_file.c_str());
		exit(-1);
	}
	
	uint64_t dc[5]; //includes the total time
	double dt[5], DT[5], DC[5]; 
	string cmd, output;
	
	eval_file = bench_file;	
	if(pid > 0) eval_file += "_A";
	else eval_file += "_B";
	if (vm.count("sh")) eval_file += "_sh";
	eval_file += ("_r=" + to_string(repeat) + ".csv");
		
	ofstream f_eval(eval_file.c_str(), std::ios::out);
	if (!f_eval.good()){
		perror(eval_file.c_str());
		exit(-1);
	}	
	
	if (pid > 0) f_eval << "Alice: ";
	else f_eval << "Bob: ";
	if (vm.count("sh")){
		f_eval << "Semi-honest settings: r = " << repeat << endl;
		if(pid == 0) cout << "Semi-honest settings: r = " << repeat << endl;
		f_eval << "Averaged over " << num_eval << " executions" << endl;
		if (pid == 0) cout << "Averaged over " << num_eval << " executions" << endl;
		f_eval << "Bench," << "Bit-width," << "Setup,," << "Init,," << "Labels,," << "Online,," << "Total,," << "Memory," << "Data" << endl;
		if (pid == 0) cout << "Bench\t" << "Bit-width\t" << "Setup\t\t\t" << "Init\t\t\t" << "Labels\t\t\t" << "Online\t\t\t" << "Total\t\t\t" << "Memory\t" << "Data" << endl;
		f_eval << "," << "," << "cc,ms," << "cc,ms," << "cc,ms," << "cc,ms,"  << "cc,ms," << "MB," << "MB" << endl;
		if (pid == 0) cout << "\t" << "\t" << "\tcc\t\tms\t" << "cc\t\tms\t" << "cc\t\tms\t" << "cc\t\tms\t"  << "cc\t\tms\t" << "MB\t" << "MB" << endl;
	}
	else{	
		f_eval << "Malicious settings: r = " << repeat << endl;
		if(pid == 0) cout << "Malicious settings: r = " << repeat << endl;
		f_eval << "Averaged over " << num_eval << " executions" << endl;
		if (pid == 0) cout << "Averaged over " << num_eval << " executions" << endl;
		f_eval << "Bench," << "Bit-width," << "Setup,," << "Inde,," << "Dep,," << "Online,," << "Total,," << "Memory" << endl;
		if (pid == 0) cout << "Bench\t" << "Bit-width\t" << "Setup\t\t\t" << "Inde\t\t\t" << "Dep\t\t\t" << "Online\t\t\t" << "Total\t\t\t" << "Memory" << endl;
		f_eval << "," << "," << "cc,ms," << "cc,ms," << "cc,ms," << "cc,ms,"  << "cc,ms," << "MB" << endl;
		if (pid == 0) cout << "\t" << "\t" << "\tcc\t\tms\t" << "cc\t\tms\t" << "cc\t\tms\t" << "cc\t\tms\t"  << "cc\t\tms\t" << "MB" << endl;
	}
		
	
	string mode;
		
	while(true) {
		fin >> mode;
		if(mode == "terminate") break;	
		
		cmd = string(BIN_PATH) + "TinyGarble --oo" \
				+ " -p " + to_string(port) \
				+ " -s " + server_ip \
				+ " -b " + to_string(bs_mal);	
		
		if(mode == "S"){
			fin >> netlist_address >> cycles >> output_mode >> benchmark >> bit_width;		
			cmd += ( \
					  " -n " + netlist_address \
					+ " -i " + input_hex_str\
					+ " -j " + init_hex_str \
					+ " -c " + to_string(cycles) \
					+ " -r " + to_string(repeat) \
					+ " -m " + to_string(output_mode));
		}
		else if(mode == "C"){
			fin >> in_file_A >> in_file_B >> benchmark >> bit_width;
			//if (!vm.count("sh")) continue;
			if (pid > 0) in_file = in_file_A;
			else in_file = in_file_B;	
			cmd += ( \
					  " -f " + in_file \
					+ " -r " + to_string(repeat));			
		}		
		else {
			cout << "Incorrecet mode: " << mode << ". Allowed options: S: Single, C: Consecutive." << endl;
		}
		
		if (vm.count("sh")) cmd += " --sh";		
		if (pid == 0) cmd += " -k 2";
		
		memset(DC, 0, 5*sizeof(double));
		memset(DT, 0, 5*sizeof(double));
		MEMORY_USAGE = 0;
		TRANSFERRED_DATA = 0;
		
		for (int i = 0; i < num_eval; ++i){		
			output = exec(cmd.c_str());
			stringstream stream(output);
			stream >> output_hex_str;
			for (int j = 0; j < 5; ++j){
				stream >> dc[j] >> dt[j];
				DC[j] += dc[j];
				DT[j] += dt[j];
			}
			stream >> memory_usage;
			MEMORY_USAGE += memory_usage;
			if (vm.count("sh")){
				stream >> transferred_data;
				TRANSFERRED_DATA += transferred_data;
			}
		}

		for (int j = 0; j < 5; ++j){
			DC[j] = DC[j]/num_eval;
			DT[j] = DT[j]/num_eval;	
		}
		MEMORY_USAGE = MEMORY_USAGE/num_eval;
		if (vm.count("sh")) TRANSFERRED_DATA = TRANSFERRED_DATA/num_eval;
		
		f_eval << benchmark << ","  << bit_width << ",";  
		if (pid == 0) cout << benchmark << "\t"  << bit_width << "\t\t";  
		for (int j = 0; j < 5; ++j){
			f_eval << setprecision(2) << scientific << DC[j] << "," << fixed << DT[j] << ",";			
			if (pid == 0) cout << setprecision(2) << scientific << DC[j] << "\t" << fixed << DT[j] << "\t";			
		}	
		f_eval << setprecision(2) << fixed << MEMORY_USAGE << ",";
		if (vm.count("sh"))f_eval << setprecision(2) << fixed << TRANSFERRED_DATA << ",";
		if (pid == 0) cout << setprecision(2) << fixed << MEMORY_USAGE << "\t";
		if (pid == 0 && (vm.count("sh"))) cout << setprecision(2) << fixed << TRANSFERRED_DATA << "\t";
		f_eval << endl;
		if (pid == 0) cout << endl;
	}		
		
	fin.close();	
	f_eval.close();	
	
	//if(pid > 0) usleep(1000);
	wait(NULL);
	
	return 0;
}
