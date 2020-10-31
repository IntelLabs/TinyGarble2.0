#ifndef SEQUENTIAL_2PC_EXEC_H__
#define SEQUENTIAL_2PC_EXEC_H__

#include <emp-tool/emp-tool.h>
#include "tinygarble/sequential_2pc.h"
#include "tinygarble/TinyGarble_config.h"

void sequential_2pc_exec(SequentialC2PC* twopc, lmkvm* lmkvm_B, lmkvm* lmkvm_A, lmkvm* lmkvm_S, lmkvm* lmkvm_R, int party, NetIO* io, CircuitFile* cf, int cycles = 1, int repeat = 1, int output_mode = 0, bool report = false, uint64_t dc[4] = (uint64_t*)default_array, double dt[4] = default_array){
	Timer T;
	int cyc_rep = cycles*repeat;

	T.start();
	
	twopc->init(cf, cycles, cyc_rep, output_mode);	
	T.get(dc[2], dt[2]);
	if (report) cout << "dep:\t" << dc[2] << "\tcc\t" << dt[2] << "\tms" << endl;
	
	T.start();

	uint64_t tr_index = 0;
	for(int cid = 0; cid < cyc_rep; ++cid) {
		twopc->copy_input_labels(lmkvm_B, lmkvm_A, lmkvm_S);
		twopc->function_dependent_st();
		io->flush();
		twopc->evaluate();
		if((output_mode == 0) || (((cid+1)%cycles) == 0)){
			lmkvm* lmkvm_R_at_tr = lmkvm_R->at(tr_index);
			twopc->retrieve_shares(lmkvm_R_at_tr);
			tr_index += cf->n3;
			delete lmkvm_R_at_tr;
		}
	}

	T.get(dc[3], dt[3]);
	if (report) cout << "online:\t" << dc[3] << "\tcc\t" << dt[3] << "\tms" << endl;
	
	twopc->clear();	
}

#endif //SEQUENTIAL_2PC_EXEC_H__
