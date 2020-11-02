#ifndef SEQUENTIAL_EXECUTION_H__
#define SEQUENTIAL_EXECUTION_H__

#include <emp-tool/emp-tool.h>
#include <cstdlib>
#include <fstream>
#include "tinygarble/sequential_2pc_exec.h"
#include "tinygarble/TinyGarble_config.h"

using namespace std;

#define BS_MAL_MIN 320

bool comp(int a, int b) 
{ 
    return (a < b); 
} 

string sequential_execution(int party, NetIO* io, string netlist_address, string input_hex_str = "", string init_hex_str = "", int cycles = 1, int repeat = 1, int output_mode = 0, int bs_mal = 0, bool report = true, uint64_t dc[4] = (uint64_t*)default_array, double dt[4] = default_array) {
	Timer T;
	
	T.start();	

	CircuitFile cf(netlist_address.c_str(), true);	
	int num_ands = 0;
	for(int i = 0; i < cf.num_gate; ++i) {
		if (cf.gates[4*i+3] == AND_GATE)
			++num_ands;
	}
	int num_inputs = cf.n1 + cf.n2;	
	int cyc_rep = cycles*repeat;
	int NUM_INPUTS = NUM_CONST + num_inputs*cyc_rep;
	int NUM_ANDS = num_ands*cyc_rep;
	
	int total_ANDS = min(NUM_ANDS, max({BS_MAL_MIN, bs_mal, num_ands}, comp));
	int total_PRE = min(NUM_INPUTS + NUM_ANDS, bs_mal);
	
	if (report) cout << total_ANDS << " " << total_PRE << endl;	
	
	SequentialC2PC* twopc = new SequentialC2PC(io, party, total_PRE, total_ANDS);
	io->flush();

	T.get(dc[0], dt[0]);
	if (report) cout << "one:\t" << dc[0] << "\tcc\t" << dt[0] << "\tms" << endl;

	T.start();
	twopc->function_independent();
	io->flush();
	twopc->new_const_labels();
	io->flush();
	
	lmkvm* lmkvm_B = new lmkvm(cyc_rep*cf.n1);	
	lmkvm* lmkvm_A = new lmkvm(cyc_rep*cf.n2);	
	twopc->new_input_labels(cf.n1, cf.n1_0, cf.n2, cf.n2_0, cycles, cyc_rep, input_hex_str, init_hex_str, lmkvm_B, lmkvm_A);
	io->flush();
	T.get(dc[1], dt[1]);
	if (report) cout << "inde:\t" << dc[1] << "\tcc\t" << dt[1] << "\tms" << endl;

	int output_bit_width = cf.n3;		
	lmkvm* lmkvm_R;
	if (output_mode == 0) lmkvm_R = new lmkvm(cyc_rep*output_bit_width);
	else lmkvm_R = new lmkvm((cyc_rep/cycles)*output_bit_width);
	sequential_2pc_exec(twopc, lmkvm_B, lmkvm_A, nullptr, lmkvm_R, party, io, &cf, cycles, repeat, output_mode, report, dc, dt);

	InputOutput* InOut = new InputOutput(0);	

	InOut->init("", 0, output_bit_width, "", 0, cycles);			
	bool *out = new bool[output_bit_width];
	memset(out, false, output_bit_width*sizeof(bool));

	uint64_t tr_index = 0;
	for(int cid = 0; cid < cyc_rep; ++cid) {
		if((output_mode == 0) || (((cid+1)%cycles) == 0)){
			lmkvm* lmkvm_R_at_tr = lmkvm_R->at(tr_index);
			twopc->reveal(output_bit_width, out, lmkvm_R_at_tr);
			InOut->fill_output(out);
			tr_index += output_bit_width;
			delete lmkvm_R_at_tr;
		}
	}
	
	string output_hex_str = InOut->read_output();

	delete[] out;		
	delete lmkvm_B;
	delete lmkvm_A;
	delete lmkvm_R;	

	delete InOut;
	delete twopc;
	
	return output_hex_str;
}

string sequential_execution(int party, NetIO* io, string in_file, int repeat_0 = 1, int bs_mal = 0, bool report = true, uint64_t dc[4] = (uint64_t*)default_array, double dt[4] = default_array) {
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
	
	int NUM_INPUTS = NUM_CONST, max_num_inputs = 0, NUM_ANDS = 0, max_num_ands = 0;
	
	while(true) {
		fin >> netlist_address;
		if(netlist_address == "terminate") break;
		fin >> input_hex_str >> init_hex_str >> cycles >> repeat_1 >> output_mode;
		repeat = repeat_0*repeat_1;
		CircuitFile cf(netlist_address.c_str(), true);	

		int num_ands = 0;
		for(int i = 0; i < cf.num_gate; ++i) {
			if (cf.gates[4*i+3] == AND_GATE)
				++num_ands;
		}
		int num_inputs = cf.n1 + cf.n2;	
		int cyc_rep = cycles*repeat;
		NUM_INPUTS += num_inputs*cyc_rep;
		max_num_inputs = max({max_num_inputs, cf.n1*cyc_rep, cf.n2*cyc_rep}, comp);
		NUM_ANDS += num_ands*cyc_rep;
		max_num_ands = max(max_num_ands, num_ands);
	}	
	
	int total_ANDS = min(NUM_ANDS, max({BS_MAL_MIN, bs_mal, max_num_ands}, comp));
	int total_PRE = min(NUM_INPUTS + NUM_ANDS, bs_mal);

	if (report) cout << "total_PRE = " << total_PRE << " total_ANDS= " << total_ANDS << endl;
	
	SequentialC2PC* twopc = new SequentialC2PC(io, party, total_PRE, total_ANDS);
	io->flush();

	T.get(dc[0], dt[0]);
	if (report) cout << "one:\t" << dc[0] << "\tcc\t" << dt[0] << "\tms" << endl;

	T.start();
	twopc->function_independent();
	io->flush();
	twopc->new_const_labels();
	io->flush();
	T.get(dc[1], dt[1]);
	if (report) cout << "inde:\t" << dc[1] << "\tcc\t" << dt[1] << "\tms" << endl;

	lmkvm* lmkvm_B;
	lmkvm* lmkvm_A;
	lmkvm* lmkvm_S = nullptr;
	lmkvm* lmkvm_R;

	InputOutput* InOut = new InputOutput(0);
	
	uint64_t dc_[4];
	double dt_[4];
	memset(dc_, 0, 4*sizeof(uint64_t));
	memset(dt_, 0, 4*sizeof(double));	

	fin.seekg(0);

	while(true) {
		fin >> netlist_address;
		if(netlist_address == "terminate") break;
		fin >> input_hex_str >> init_hex_str >> cycles >> repeat_1 >> output_mode;
		repeat = repeat_0*repeat_1;
		if (report) cout << netlist_address << " -i " << input_hex_str << " -j " << init_hex_str << " -c " << cycles << " -r " << repeat << " -m " << output_mode << endl;

		CircuitFile cf(netlist_address.c_str(), true);
		if((cf.n0) && (lmkvm_S == nullptr)){
			cout << netlist_address << " expects a shared input according to the config file " << in_file << endl;
			cout << " Please make sure the previous netslist execution have the output mode as 2 or 3" << endl;
			exit(-1);
		}
		int cyc_rep = cycles*repeat;

		T.start();
		lmkvm_B = new lmkvm(cyc_rep*cf.n1);	
		lmkvm_A = new lmkvm(cyc_rep*cf.n2);	
		twopc->new_input_labels(cf.n1, cf.n1_0, cf.n2, cf.n2_0, cycles, cyc_rep, input_hex_str, init_hex_str, lmkvm_B, lmkvm_A);
		io->flush();
		T.get(dc_[1], dt_[1]);

		int output_bit_width = cf.n3;	
		if ((output_mode == 0)||(output_mode == 2)) lmkvm_R = new lmkvm(cyc_rep*output_bit_width);
		else lmkvm_R = new lmkvm((cyc_rep/cycles)*output_bit_width);
		sequential_2pc_exec(twopc, lmkvm_B, lmkvm_A, lmkvm_S, lmkvm_R, party, io, &cf, cycles, repeat, output_mode, report, dc_, dt_);

		InOut->init("", 0, output_bit_width, "", 0, cycles);			
		bool *out = new bool[output_bit_width];
		memset(out, false, output_bit_width*sizeof(bool));

		uint64_t tr_index = 0;
		for(int cid = 0; cid < cyc_rep; ++cid) {
			if( (output_mode == 0) || (output_mode == 1) && (((cid+1)%cycles) == 0) ){
				lmkvm* lmkvm_R_at_tr = lmkvm_R->at(tr_index);
				twopc->reveal(output_bit_width, out, lmkvm_R_at_tr);
				InOut->fill_output(out);
				tr_index += output_bit_width;
				delete lmkvm_R_at_tr;
			}
			else if ( (output_mode == 2) || (output_mode == 3) && (((cid+1)%cycles) == 0) ){
				if (lmkvm_S != nullptr){
					delete lmkvm_S;
					lmkvm_S = nullptr;
				}	
				if (output_mode == 2) lmkvm_S = new lmkvm(cyc_rep*output_bit_width);
				else lmkvm_S = new lmkvm((cyc_rep/cycles)*output_bit_width);
				lmkvm_S->copy(lmkvm_R);
			}
		}

		delete[] out;		
		delete lmkvm_B;
		delete lmkvm_A;
		delete lmkvm_R;	
		
		for (int j = 1; j < 4; ++j){
			dc[j] += dc_[j];
			dt[j] += dt_[j];
		}
	}
		
	fin.close();

	output_hex_str = InOut->read_output();

	delete InOut;
	delete twopc;
	
	return output_hex_str;
}

#endif //SEQUENTIAL_EXECUTION_H__
