#ifndef SEQUENTIAL_EXECUTION_SH_H__
#define SEQUENTIAL_EXECUTION_SH_H__

#include <emp-tool/emp-tool.h>
#include "tinygarble/sequential_2pc_exec_sh.h"

string sequential_execution_sh(int party, NetIO* io, string netlist_address, string input_hex_str = "", string init_hex_str = "", int cycles = 1, int repeat = 1, int output_mode = 0, bool report = true, uint64_t dc[4] = (uint64_t*)default_array, double dt[4] = default_array) {
	Timer T;
	
	T.start();	
	InputOutput* InOut = new InputOutput(0);	
	SequentialC2PC_SH* twopc = new SequentialC2PC_SH(io, party);
	io->flush();
	CircuitFile cf(netlist_address.c_str(), true);	
	T.get(dc[0], dt[0]);
	if (report) cout << "one:\t" << dc[0] << "\tcc\t" << dt[0] << "\tms" << endl;
		
	
	T.start();
	int cyc_rep = cycles*repeat;
	block* labels_B = new block[cyc_rep*cf.n1];
	block* labels_A = new block[cyc_rep*cf.n2];
	block* labels_S = nullptr;
	block* labels_R;
	twopc->gen_input_labels(cf.n1, cf.n1_0, labels_B, cycles, cyc_rep, cf.n2, cf.n2_0, labels_A, cycles, cyc_rep, input_hex_str, init_hex_str);	
	T.get(dc[2], dt[2]);
	if (report) cout << "labels:\t" << dc[2] << "\tcc\t" << dt[2] << "\tms" << endl;

	sequential_2pc_exec_sh(InOut, twopc, labels_B, labels_A, labels_S, labels_R, party, io, &cf, cycles, repeat, output_mode, report, dc, dt);
	
	string output_hex_str = InOut->read_output();
	
	delete [] labels_B;
	delete [] labels_A;
	delete InOut;
	delete twopc;
	
	return output_hex_str;
}

string sequential_execution_sh(int party, NetIO* io, string in_file, int repeat_0 = 1, bool report = true, uint64_t dc[4] = (uint64_t*)default_array, double dt[4] = default_array) {	
	Timer T;	
	memset(dc, 0, 4*sizeof(uint64_t));
	memset(dt, 0, 4*sizeof(double));
	T.start();
	
	string netlist_address, output_hex_str, input_hex_str = "0", init_hex_str = "0";
	int cycles = 1, repeat = 1, repeat_1 = 1, output_mode = 0, old_output_mode = 0;
	
	ifstream fin(in_file.c_str(), std::ios::in);
	if (!fin.good()){
		perror(in_file.c_str());
		exit(-1);
	}
	
	InputOutput* InOut = new InputOutput(0);
	
	SequentialC2PC_SH* twopc = new SequentialC2PC_SH(io, party);
	io->flush();	
	T.get(dc[0], dt[0]);
	if (report) cout << "one:\t" << dc[0] << "\tcc\t" << dt[0] << "\tms" << endl;
	
	block* labels_S = nullptr;
	block* labels_R;
	
	uint64_t dc_[4];
	double dt_[4];
	memset(dc_, 0, 4*sizeof(uint64_t));
	memset(dt_, 0, 4*sizeof(double));	
		
	while(true) {
		fin >> netlist_address;
		if(netlist_address == "terminate") break;
		fin >> input_hex_str >> init_hex_str >> cycles >> repeat_1 >> output_mode;
		repeat = repeat_0*repeat_1;
		if (report) cout << netlist_address << " -i " << input_hex_str << " -j " << init_hex_str << " -c " << cycles << " -r " << repeat << " -m " << output_mode << endl;
		
		CircuitFile cf(netlist_address.c_str(), true);
		if((cf.n0) && (labels_S == nullptr)){
			cout << netlist_address << " expects a shared input according to the config file " << in_file << endl;
			cout << " Please make sure the previous netslist execution have the output mode as 2 or 3" << endl;
			exit(-1);
		}

		T.start();
		int cyc_rep = cycles*repeat;
		block* labels_B = new block[cyc_rep*cf.n1];
		block* labels_A = new block[cyc_rep*cf.n2];
		twopc->gen_input_labels(cf.n1, cf.n1_0, labels_B, cycles, cyc_rep, cf.n2, cf.n2_0, labels_A, cycles, cyc_rep, input_hex_str, init_hex_str);	
		T.get(dc_[2], dt_[2]);
		if (report) cout << "labels:\t" << dc_[2] << "\tcc\t" << dt_[2] << "\tms" << endl;
		
		if (output_mode == 2) labels_R = new block[cyc_rep*cf.n3];
		else if (output_mode == 3) labels_R = new block[(cyc_rep/cycles)*(cf.n3)];
		
		sequential_2pc_exec_sh(InOut, twopc, labels_B, labels_A, labels_S, labels_R, party, io, &cf, cycles, repeat, output_mode, report, dc_, dt_);

		if ((old_output_mode == 2) || (old_output_mode == 3)){
			delete [] labels_S;	
			labels_S = nullptr;	
		}
		if (output_mode == 2) {
			labels_S = new block[cyc_rep*cf.n3];
			memcpy(labels_S, labels_R, cyc_rep*cf.n3*sizeof(block));
			delete [] labels_R;
		}
		else if (output_mode == 3) {
			labels_S = new block[(cyc_rep/cycles)*(cf.n3)];
			memcpy(labels_S, labels_R, (cyc_rep/cycles)*(cf.n3)*sizeof(block));
			delete [] labels_R;
		}	
		old_output_mode = output_mode;
		
		delete [] labels_B;
		delete [] labels_A;
		
		for (int j = 1; j < 4; ++j){
			dc[j] += dc_[j];
			dt[j] += dt_[j];
		}
	}	
		
	fin.close();
	
	output_hex_str = InOut->read_output();
	
	if (labels_S != nullptr) delete [] labels_S;	
	delete InOut;
	delete twopc;
	
	return output_hex_str;
}

#endif //SEQUENTIAL_EXECUTION_SH_H__
