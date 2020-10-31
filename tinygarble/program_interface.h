#ifndef PROGRAM_INTERFACE_H
#define PROGRAM_INTERFACE_H

#include <cstdlib>
#include <math.h>
#include <vector>
#include <numeric>
#include <string>
#include <emp-tool/emp-tool.h>
#include "sequential_2pc.h"
#include "sequential_2pc_exec.h"
#include "TinyGarble_config.h"
#include "helper.h"

using namespace std;

class TinyGarblePI{
	public:
	NetIO* io;
	int party;	
	SequentialC2PC* twopc;
	
	vector<int64_t> input, output;
	vector<uint64_t> bit_width_A, bit_width_B;	
	
	lmkvm* lmkvm_A;
	lmkvm* lmkvm_B;
	
	uint64_t retreived_index_A = 0, retreived_index_B = 0;
	
	TinyGarblePI(NetIO* io, int party, int total_PRE = 10000, int total_ANDS = 10000) {
		this->party = party;
		this->io = io;		
		twopc = new SequentialC2PC(io, party, total_PRE, total_ANDS);
		io->flush();
		twopc->function_independent();
		io->flush();
		twopc->new_const_labels();
		io->flush();
	}	

	/***register inputs from Alice or Bob, must be followed by gen_input_labels and retrieve_input_labels***/
	
	void register_input(int owner, uint64_t bit_width, int64_t val = 0){
	/*Register input of a particular owner, must be called before gen_input_labels()
	val is the secret value of the input known to the owner, val is ignored if the input is owned by the other party*/
		if(owner == party) input.push_back(val);
		if(owner == ALICE) bit_width_A.push_back(bit_width);
		else bit_width_B.push_back(bit_width);
	}
	
	void register_input_vector(int owner, uint64_t bit_width, auto val, uint64_t len0){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			register_input(owner, bit_width, val[i0]);
	}
	void register_input_vector(int owner, uint64_t bit_width, auto val, uint64_t len0, uint64_t len1){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				register_input(owner, bit_width, val[i0][i1]);
	}
	void register_input_vector(int owner, uint64_t bit_width, auto val, uint64_t len0, uint64_t len1, uint64_t len2){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					register_input(owner, bit_width, val[i0][i1][i2]);
	}
	void register_input_vector(int owner, uint64_t bit_width, auto val, uint64_t len0, uint64_t len1, uint64_t len2, uint64_t len3){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					for (uint64_t i3 = 0; i3 < len3; i3++)
						register_input(owner, bit_width, val[i0][i1][i2][i3]);
	}

	/***assign public or secret value to a secret variable***/
	
	void sign_extend(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width_target, uint64_t bit_width){
		y_x->copy(a_x, 0, bit_width);
		for (uint64_t i = bit_width; i < bit_width_target; i++)
			y_x->copy(a_x->att(bit_width-1), i, 1);
	}
	void unsign_extend(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width_target, uint64_t bit_width){
		y_x->copy(a_x, 0, bit_width);
		for (uint64_t i = bit_width; i < bit_width_target; i++)
			y_x->copy(twopc->const_lmkvm->att(0), i, 1);
	}
	
	void assign(lmkvm* y_x, int64_t val, uint64_t bit_width){
	/*y = val, val is known to both parties, bit_width <= 64*/
		bitset<64> bin(val);
		for (uint64_t i = 0; i < bit_width; i++){
			y_x->copy(twopc->const_lmkvm->att((uint64_t)bin[i]), i, 1);
		}
	}	
	void assign(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width){
	/*y = a_x*/
		y_x->copy(a_x, 0, bit_width);
	}	
	void assign(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width_y, uint64_t bit_width_a){
	/*y = a_x*/
		if(bit_width_a < bit_width_y){
			lmkvm* a1_x = new lmkvm(bit_width_y);
			sign_extend(a1_x, a_x, bit_width_y, bit_width_a);
			assign(y_x, a1_x, bit_width_y);
			delete a1_x;
		}
		else assign(y_x, a_x, bit_width_y);
	}

	void assign_vector(auto& A_x, auto A, uint64_t bit_width, uint64_t len0) {	
		for(uint64_t i0 = 0; i0 < len0; i0++)
			assign(A_x[i0], A[i0], bit_width);				
	}
	void assign_vector(auto& A_x, auto A, uint64_t bit_width, uint64_t len0, uint64_t len1) {
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				assign(A_x[i0][i1], A[i0][i1], bit_width);
	}
	void assign_vector(auto& A_x, auto A, uint64_t bit_width, uint64_t len0, uint64_t len1, uint64_t len2) {
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					assign(A_x[i0][i1][i2], A[i0][i1][i2], bit_width);
	}		
	void assign_vector(auto& A_x, auto A, uint64_t bit_width, uint64_t len0, uint64_t len1, uint64_t len2, uint64_t len3){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					for (uint64_t i3 = 0; i3 < len3; i3++)
						assign(A_x[i0][i1][i2][i3], A[i0][i1][i2][i3], bit_width);
	}
	
	/***create secret variable***/
	
	lmkvm* TG_int(uint64_t bit_width){
		lmkvm* Z;
		Z = new lmkvm(bit_width);
		return Z;
	}
	vector<lmkvm*> TG_int(uint64_t bit_width, size_t len0){
		vector<lmkvm*> Z(len0);
		for (uint64_t i0 = 0; i0 < len0; i0++)
			Z[i0] = new lmkvm (bit_width);
		return Z;
	}
	vector<vector<lmkvm*>> TG_int(uint64_t bit_width, size_t len0, size_t len1){
		vector<vector<lmkvm*>> Z(len0, vector<lmkvm*>(len1));
		for (uint64_t i0 = 0; i0 < len0; i0++)
			for (uint64_t i1 = 0; i1 < len1; i1++)
				Z[i0][i1] = new lmkvm (bit_width);
		return Z;
	}
	vector<vector<vector<vector<lmkvm*>>>> TG_int(uint64_t bit_width, size_t len0, size_t len1, size_t len2, size_t len3){
		vector<vector<vector<vector<lmkvm*>>>> Z(len0, std::vector<vector<vector<lmkvm*>>>(len1, vector<vector<lmkvm*>>(len2, std::vector<lmkvm*>(len3))));
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					for (uint64_t i3 = 0; i3 < len3; i3++)
						Z[i0][i1][i2][i3] = new lmkvm (bit_width);
		return Z;
	}

	/***create secret variable and register input or assign public value***/
	
	lmkvm* TG_int_init(int owner, uint64_t bit_width, auto val){
		auto Z = TG_int(bit_width);
		
		if (owner == PUBLIC)
			assign(Z, val, bit_width);
		else
			register_input(owner, bit_width, val);
		
		return Z;
	}
	vector<lmkvm*> TG_int_init(int owner, uint64_t bit_width, auto val, size_t len0){
		auto Z = TG_int(bit_width, len0);
		
		if(owner == PUBLIC)
			assign_vector(Z, val, bit_width, len0);
		else
			register_input_vector(owner, bit_width, val, len0);
		
		return Z;
	}
	vector<vector<lmkvm*>> TG_int_init(int owner, uint64_t bit_width, auto val, size_t len0, size_t len1){
		auto Z = TG_int(bit_width, len0, len1);
		
		if(owner == PUBLIC)
			assign_vector(Z, val, bit_width, len0, len1);
		else
			register_input_vector(owner, bit_width, val, len0, len1);
		
		return Z;
	}
	vector<vector<vector<vector<lmkvm*>>>> TG_int_init(int owner, uint64_t bit_width, auto val, size_t len0, size_t len1, size_t len2, size_t len3){
		auto Z = TG_int(bit_width, len0, len1, len2, len3);
		
		if(owner == PUBLIC)
			assign_vector(Z, val, bit_width, len0, len1, len2, len3);
		else
			register_input_vector(owner, bit_width, val, len0, len1, len2, len3);
		
		return Z;
	}

	/***clear memory alloacted to secret variable***/

	void clear_TG_int(auto& A){
		delete A;
	}
	void clear_TG_int(auto& A, uint64_t len0){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			delete A[i0];
		A.clear();
		A.shrink_to_fit();
	}	
	void clear_TG_int(auto& A, uint64_t len0, uint64_t len1){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				delete A[i0][i1];
		A.clear();
		A.shrink_to_fit();
	}
	void clear_TG_int(auto& A, uint64_t len0, uint64_t len1, uint64_t len2){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					delete A[i0][i1][i2];
		A.clear();
		A.shrink_to_fit();
	}
	void clear_TG_int(auto& A, uint64_t len0, uint64_t len1, uint64_t len2, uint64_t len3){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					for (uint64_t i3 = 0; i3 < len3; i3++)
						delete A[i0][i1][i2][i3];
		A.clear();
		A.shrink_to_fit();
	}

	/***generate input labels and retrieve to corresponding secret variables***/
	
	void gen_input_labels(){
	/*Generate and transfer the labels for inputs of both parties, includes OT*/
		reverse(input.begin(), input.end()); 
		reverse(bit_width_A.begin(), bit_width_A.end()); 
		reverse(bit_width_B.begin(), bit_width_B.end()); 
		
		uint64_t n1 = accumulate(bit_width_B.begin(), bit_width_B.end(), 0), n1_0 = 0;
		uint64_t n2 = accumulate(bit_width_A.begin(), bit_width_A.end(), 0), n2_0 = 0;
		uint64_t n = (party == ALICE)? n2: n1;
		uint64_t cycles_B = 1, cyc_rep_B = 1, cycles_A = 1, cyc_rep_A = 1;
		vector<uint64_t> bit_width = (party == ALICE)? bit_width_A: bit_width_B;
		
		bool *IN = new bool[n];
		dec_vector_to_bin(IN, input, bit_width);
		
		lmkvm_B = new lmkvm(cyc_rep_B*n1);
		lmkvm_A = new lmkvm(cyc_rep_A*n2);
		
		twopc->new_input_labels(n1, n2, lmkvm_B, lmkvm_A, IN);
		
		input.clear();
		input.shrink_to_fit();
		bit_width_A.clear();
		bit_width_A.shrink_to_fit();
		bit_width_B.clear();
		bit_width_B.shrink_to_fit();
		bit_width.clear();
		bit_width.shrink_to_fit();
		delete [] IN;
	}
	
	void clear_input_labels(){
	/*clear memory for generated input labels, should only be called after all the labels are retrieved*/
		delete lmkvm_A;
		delete lmkvm_B;
		retreived_index_A = 0;
		retreived_index_B = 0;
	}
	
	void retrieve_input_labels(lmkvm* retreived_lmkvm, int owner, uint64_t bit_width){
	/*Labels of input have to be retrieved in the same order they were registered with register_input(), must be called after gen_input_labels()*/
		if(owner == ALICE){ 
			retreived_lmkvm->copy(lmkvm_A->att(retreived_index_A), 0, bit_width);
			retreived_index_A += bit_width;
		}
		else {
			retreived_lmkvm->copy(lmkvm_B->att(retreived_index_B), 0, bit_width);
			retreived_index_B += bit_width;
		}
	}		

	void retrieve_input_vector_labels(auto& retreived_vector_labels, int owner, uint64_t bit_width, uint64_t len0) {
		retreived_vector_labels = TG_int(bit_width, len0);	
		for(uint64_t i0 = 0; i0 < len0; i0++)
			retrieve_input_labels(retreived_vector_labels[i0], owner, bit_width);				
	}
	void retrieve_input_vector_labels(auto& retreived_vector_labels, int owner, uint64_t bit_width, uint64_t len0, uint64_t len1) {
		retreived_vector_labels = TG_int(bit_width, len0, len1);
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				retrieve_input_labels(retreived_vector_labels[i0][i1], owner, bit_width);
	}		
	void retrieve_input_vector_labels(auto& retreived_vector_labels, int owner, uint64_t bit_width, uint64_t len0, uint64_t len1, uint64_t len2, uint64_t len3){
		retreived_vector_labels = TG_int(bit_width, len0, len1, len2, len3);
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					for (uint64_t i3 = 0; i3 < len3; i3++)
						retrieve_input_labels(retreived_vector_labels[i0][i1][i2][i3], owner, bit_width);
	}	

	/***reveal secret variable***/
	
	int64_t reveal(lmkvm* lmkvm_R, uint64_t bit_width, bool is_signed = true){
	/*Combine shares to reveal the secret value of an integer*/
		InputOutput* InOut = new InputOutput(0);
		InOut->init("", 0, bit_width, "", 0, 1);		
		bool *out = new bool[bit_width];
		memset(out, false, bit_width*sizeof(bool));
		
		twopc->reveal(bit_width, out, lmkvm_R);			
		InOut->fill_output(out);
		string output_hex_str = InOut->read_output();
		
		vector<int64_t> output;	
		parseGCOutputString(output, output_hex_str, bit_width, 0, is_signed);
		
		delete InOut;
		delete[] out;
		return output.back();
	}
	int64_t reveal(lmkvm* lmkvm_R){
		return reveal(lmkvm_R, lmkvm_R->num_bits);
	}
	
	void reveal_vector(auto& revealed_vector, auto vector_labels, uint64_t bit_width, uint64_t len0) {
		revealed_vector = make_vector<int64_t>(len0);	
		for(uint64_t i0 = 0; i0 < len0; i0++){
			revealed_vector[i0] = reveal(vector_labels[i0], bit_width);
		}				
	}
	void reveal_vector(auto& revealed_vector, auto vector_labels, uint64_t bit_width, uint64_t len0, uint64_t len1) {
		revealed_vector = make_vector<int64_t>(len0, len1);
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++){
				revealed_vector[i0][i1] = reveal(vector_labels[i0][i1], bit_width);
			}
	}
	void reveal_vector(auto& revealed_vector, auto vector_labels, uint64_t bit_width, uint64_t len0, uint64_t len1, uint64_t len2) {
		revealed_vector = make_vector<int64_t>(len0, len1, len2);
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++){
					revealed_vector[i0][i1][i2] = reveal(vector_labels[i0][i1][i2], bit_width);
				}
	}		
	void reveal_vector(auto& revealed_vector, auto vector_labels, uint64_t bit_width, uint64_t len0, uint64_t len1, uint64_t len2, uint64_t len3) {
		revealed_vector = make_vector<int64_t>(len0, len1, len2, len3);
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					for (uint64_t i3 = 0; i3 < len3; i3++){
						revealed_vector[i0][i1][i2][i3] = reveal(vector_labels[i0][i1][i2][i3], bit_width);
					}
	}

	/***arithmetic and logical operations***/
	
	void fun(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width, string op){
		string netlist_address = string(NETLIST_PATH_PI) + op + "_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void fun(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b, string op){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		lmkvm* a1_x = new lmkvm(bit_width);
		lmkvm* b1_x = new lmkvm(bit_width);
		sign_extend(a1_x, a_x, bit_width, bit_width_a);
		sign_extend(b1_x, b_x, bit_width, bit_width_b);
		fun(y_x, a1_x, b1_x, bit_width, op);
		delete a1_x;
		delete b1_x;
	}
	void fun(lmkvm* y_x, lmkvm* a_x, int64_t b, uint64_t bit_width, string op){
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		fun(y_x, a_x, b_x, bit_width, op);
		delete b_x;
	}
	
	/*y = a + b*/
	void add(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "add");
	}	
	void add(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "add");
	}

	/*y = a - b*/
	void sub(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "sub");
	}	
	void sub(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "sub");
	}
	
	/*y = a < b*/
	void lt(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "lt");
	}	
	void lt(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "lt");
	}
	
	/*y = max(a, b)*/
	void max(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "max");
	}	
	void max(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "max");
	}
	
	/*y = min(a, b)*/
	void min(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "min");
	}	
	void min(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "min");
	}
	
	void logic(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width, string op){
		string netlist_address = string(NETLIST_PATH_PI) + op + "_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void logic(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b, string op){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		lmkvm* a1_x = new lmkvm(bit_width);
		lmkvm* b1_x = new lmkvm(bit_width);
		unsign_extend(a1_x, a_x, bit_width, bit_width_a);
		unsign_extend(b1_x, b_x, bit_width, bit_width_b);
		logic(y_x, a1_x, b1_x, bit_width, op);
		delete a1_x;
		delete b1_x;
	}
	
	/*y = a & b*/
	void and_(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "and");
	}	
	void and_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "and");
	}
	void and_(lmkvm* y_x, lmkvm* a_x, uint64_t b, uint64_t bit_width){
		bitset<64> bits(b);
		for (uint64_t i = 0; i < bit_width; i++)
			if (bits[i]) y_x->copy(a_x->att(i), i, 1);	
			else y_x->copy(twopc->const_lmkvm->att(0), i, 1);
	}
	
	/*y = a | b*/
	void or_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "or");
	}	
	void or_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "or");
	}
	void or_(lmkvm* y_x, lmkvm* a_x, uint64_t b, uint64_t bit_width){
		bitset<64> bits(b);
		for (uint64_t i = 0; i < bit_width; i++)
			if (bits[i]) y_x->copy(twopc->const_lmkvm->att(1), i, 1);	
			else y_x->copy(a_x->att(i), i, 1);
	}
	
	/*y = a ^ b*/
	void xor_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "xor");
	}	
	void xor_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "xor");
	}
	void xor_(lmkvm* y_x, lmkvm* a_x, uint64_t b, uint64_t bit_width){
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		xor_(y_x, a_x, b_x, bit_width);
		delete b_x;
	}
	
	/*y = -a*/
	void neg(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width){
		lmkvm* zero = TG_int_init(PUBLIC, bit_width, (int64_t)0);
		sub(y_x, zero, a_x, bit_width);
		delete zero;
	}

	/*y = ~a*/
	void not_(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width){
		lmkvm* one = new lmkvm(bit_width);
		for (uint64_t i = 0; i < bit_width; i++){
			one->copy(twopc->const_lmkvm->att(1), i, 1);
		}
		xor_(y_x, one, a_x, bit_width);
		delete one;
	}

	/*y = a * b*/
	void mult(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		string netlist_address = string(NETLIST_PATH_PI) + "mult_" + to_string(bit_width_a) + "_" + to_string(bit_width_b) + "_" + to_string(bit_width_a+bit_width_b-1) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}	
	void mult(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		mult(y_x, a_x, b_x, bit_width, bit_width);
	}
	void mult(lmkvm* y_x, lmkvm* a_x, int64_t b, uint64_t bit_width_a, uint64_t bit_width_b){
		uint64_t bit_width_y = bit_width_a + bit_width_b - 1;
		assign(y_x, (int64_t)0, bit_width_y);
		auto a1_x = TG_int(bit_width_a);
		if(b < 0) {
			neg(a1_x, a_x, bit_width_a);
			b = -b;
		}
		else assign(a1_x, a_x, bit_width_a);
		auto a2_x = TG_int(bit_width_y);
		assign(a2_x, a1_x, bit_width_y, bit_width_a);
		bitset<64> bits(b);
		for (uint64_t i = 0; i < bit_width_b; i++){
			if (bits[i]) add(y_x, y_x, a2_x, bit_width_y);
			left_shift(a2_x, 1, bit_width_y);
		}
		delete a1_x;
		delete a2_x;
	}
	
	void mat_mult(uint64_t row_A, uint64_t inner, uint64_t col_B, auto &A, auto &B, auto &C, int64_t rs_bits, uint64_t bit_width_A, uint64_t bit_width_B, uint64_t bit_width_C, uint64_t bit_width_G){
	
		string netlist_address = string(NETLIST_PATH_PI) + "mac_" + to_string(bit_width_A) + "_" + to_string(bit_width_B) + "_" + to_string(bit_width_G) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = inner, repeat = 1, output_mode = 3;
		
		lmkvm* lmkvm_A = new lmkvm(row_A*inner*bit_width_A);
		lmkvm* lmkvm_B = new lmkvm(inner*col_B*bit_width_B);
		lmkvm* lmkvm_C = new lmkvm(bit_width_G);
		
		for (uint64_t i = 0; i < row_A; i++)
			for (uint64_t j = 0; j < inner; j++)
				for (uint64_t k = 0; k < bit_width_A; k++)	
					lmkvm_A->copy(A[i][j]->att(k), (i*inner + j)*bit_width_A + k, 1);
			
		for (uint64_t i = 0; i < col_B; i++)
			for (uint64_t j = 0; j < inner; j++)
				for (uint64_t k = 0; k < bit_width_B; k++)
					lmkvm_B->copy(B[j][i]->att(k), (i*inner + j)*bit_width_B + k, 1);
			
		for (uint64_t i = 0; i < row_A; i++){
			for (uint64_t j = 0; j < col_B; j++){
				sequential_2pc_exec(twopc, lmkvm_B->at(j*inner*bit_width_B), lmkvm_A->at(i*inner*bit_width_A), nullptr, lmkvm_C, party, io, &cf, cycles, repeat, output_mode);
				right_shift(lmkvm_C, rs_bits, bit_width_G);
				assign(C[i][j], lmkvm_C, bit_width_C);
				cout << fixed << setprecision(2) << setfill('0');
				cout << "\r   " << ((double)(i*col_B+j+1))/(row_A*col_B)*100 << "%";
			}
		}
		cout << endl;
		
		delete lmkvm_A;
		delete lmkvm_B;
		delete lmkvm_C;
	}
	
	/*y = a / b*/
	void div(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		string netlist_address = string(NETLIST_PATH_PI) + "div_" + to_string(bit_width_a) + "_" + to_string(bit_width_b) + "_" + to_string(bit_width_a) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void div(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		div(y_x, a_x, b_x, bit_width, bit_width);
	}
		
	/*y = c? a : b*/
	void ifelse(lmkvm* y_x, lmkvm* c_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		lmkvm* ab_x = new lmkvm(2*bit_width);
		ab_x->copy(b_x, 0, bit_width);
		ab_x->copy(a_x, bit_width, bit_width);		
		string netlist_address = string(NETLIST_PATH_PI) + "ifelse_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;	
		sequential_2pc_exec(twopc, c_x, ab_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);	
		delete ab_x;
	}
	void ifelse(lmkvm* y_x, lmkvm* c_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		lmkvm* a1_x = new lmkvm(bit_width);
		lmkvm* b1_x = new lmkvm(bit_width);
		sign_extend(a1_x, a_x, bit_width, bit_width_a);
		sign_extend(b1_x, b_x, bit_width, bit_width_b);
		ifelse(y_x, c_x, a1_x, b1_x, bit_width);
		delete a1_x;
		delete b1_x;
	}
	void ifelse(lmkvm* y_x, lmkvm* c_x, lmkvm* a_x, int64_t b, uint64_t bit_width){
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		lmkvm* ab_x = new lmkvm(2*bit_width);
		ab_x->copy(b_x, 0, bit_width);
		ab_x->copy(a_x, bit_width, bit_width);		
		string netlist_address = string(NETLIST_PATH_PI) + "ifelse_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;	
		sequential_2pc_exec(twopc, c_x, ab_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);	
		delete ab_x;
		delete b_x;
	}

	/*a << shift*/
	void left_shift(lmkvm* a_x, uint64_t shift, uint64_t bit_width){
		for (int64_t i = bit_width - 1; i >= (int64_t)shift; i--){
			a_x->copy(a_x->att(i - shift), i, 1);
		}
		for (int64_t i = shift - 1; i >= 0; i--){
			a_x->copy(twopc->const_lmkvm->att(0), i, 1);
		}
	}		
	/*a >> shift*/
	void right_shift(lmkvm* a_x, uint64_t shift, uint64_t bit_width){	
		a_x->copy(a_x->att(shift), 0, bit_width - shift);
		for (uint64_t i = bit_width - shift; i < bit_width; i++){
			a_x->copy(a_x->att(bit_width - 1), i, 1);
		}
	}

	/*relu(a)*/		
	void relu(lmkvm* a_x, uint64_t bit_width){
		lmkvm* sign_x = new lmkvm(1);
		not_(sign_x, a_x->at(bit_width-1), 1);
		lmkvm* mask_x = new lmkvm(bit_width);
		for (uint64_t i = 0; i < bit_width-1; i++){
			mask_x->copy(sign_x, i, 1);
		}
		mask_x->copy(twopc->const_lmkvm->att(0), bit_width-1, 1);
		and_(a_x, a_x, mask_x, bit_width);
	}

};

#endif //PROGRAM_INTERFACE_H