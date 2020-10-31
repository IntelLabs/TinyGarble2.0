#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"

#ifndef PROGRAM_INTERFACE_SH_H
#define PROGRAM_INTERFACE_SH_H

#include <cstdlib>
#include <math.h>
#include <vector>
#include <numeric>
#include <string>
#include <emp-tool/emp-tool.h>
#include "sequential_2pc_sh.h"
#include "sequential_2pc_exec_sh.h"
#include "TinyGarble_config.h"
#include "helper.h"

using namespace std;

class TinyGarblePI_SH{
	public:
	NetIO* io;
	int party;	
	SequentialC2PC_SH* twopc;
	
	vector<int64_t> input, output;
	vector<uint64_t> bit_width_A, bit_width_B;	
	
	block* labels_A;
	block* labels_B;
	
	uint64_t retreived_index_A = 0, retreived_index_B = 0;
	
	TinyGarblePI_SH(NetIO* io, int party) {
		this->party = party;
		this->io = io;		
		twopc = new SequentialC2PC_SH(io, party);
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
	
	void sign_extend(block*& y_x, block* a_x, uint64_t bit_width_target, uint64_t bit_width){
		memcpy(y_x, a_x, bit_width*sizeof(block));
		for (uint64_t i = bit_width; i < bit_width_target; i++)
			memcpy(&y_x[i], &a_x[bit_width-1], sizeof(block));
	}
	void unsign_extend(block*& y_x, block* a_x, uint64_t bit_width_target, uint64_t bit_width){
		memcpy(y_x, a_x, bit_width*sizeof(block));
		for (uint64_t i = bit_width; i < bit_width_target; i++)
			memcpy(&y_x[i], &twopc->label_const[0], sizeof(block));
	}
	
	void assign(block*& y_x, int64_t val, uint64_t bit_width){
	/*y = val, val is known to both parties, bit_width <= 64*/
		bitset<64> bin(val);
		for (uint64_t i = 0; i < bit_width; i++){
			memcpy(&y_x[i], &twopc->label_const[(uint64_t)bin[i]], sizeof(block));
		}
	}	
	void assign(block*& y_x, block* a_x, uint64_t bit_width){
	/*y = a_x*/
		memcpy(y_x, a_x, bit_width*sizeof(block));
	}	
	void assign(block*& y_x, block* a_x, uint64_t bit_width_y, uint64_t bit_width_a){
	/*y = a_x*/
		if(bit_width_a < bit_width_y){
			block* a1_x = new block[bit_width_y];
			sign_extend(a1_x, a_x, bit_width_y, bit_width_a);
			assign(y_x, a1_x, bit_width_y);
			delete[] a1_x;
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
	
	block* TG_int(uint64_t bit_width){
		block* Z;
		Z = new block[bit_width];
		return Z;
	}
	vector<block*> TG_int(uint64_t bit_width, size_t len0){
		vector<block*> Z(len0);
		for (uint64_t i0 = 0; i0 < len0; i0++)
			Z[i0] = new block [bit_width];
		return Z;
	}
	vector<vector<block*>> TG_int(uint64_t bit_width, size_t len0, size_t len1){
		vector<vector<block*>> Z(len0, vector<block*>(len1));
		for (uint64_t i0 = 0; i0 < len0; i0++)
			for (uint64_t i1 = 0; i1 < len1; i1++)
				Z[i0][i1] = new block [bit_width];
		return Z;
	}
	vector<vector<vector<vector<block*>>>> TG_int(uint64_t bit_width, size_t len0, size_t len1, size_t len2, size_t len3){
		vector<vector<vector<vector<block*>>>> Z(len0, std::vector<vector<vector<block*>>>(len1, vector<vector<block*>>(len2, std::vector<block*>(len3))));
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					for (uint64_t i3 = 0; i3 < len3; i3++)
						Z[i0][i1][i2][i3] = new block [bit_width];
		return Z;
	}

	/***create secret variable and register input or assign public value***/
	
	block* TG_int_init(int owner, uint64_t bit_width, auto val){
		auto Z = TG_int(bit_width);
		
		if(owner == PUBLIC)
			assign(Z, val, bit_width);
		else
			register_input(owner, bit_width, val);
		
		return Z;
	}
	vector<block*> TG_int_init(int owner, uint64_t bit_width, auto val, size_t len0){
		auto Z = TG_int(bit_width, len0);
		
		if(owner == PUBLIC)
			assign_vector(Z, val, bit_width, len0);
		else
			register_input_vector(owner, bit_width, val, len0);
		
		return Z;
	}
	vector<vector<block*>> TG_int_init(int owner, uint64_t bit_width, auto val, size_t len0, size_t len1){
		auto Z = TG_int(bit_width, len0, len1);
		
		if(owner == PUBLIC)
			assign_vector(Z, val, bit_width, len0, len1);
		else
			register_input_vector(owner, bit_width, val, len0, len1);
		
		return Z;
	}
	vector<vector<vector<vector<block*>>>> TG_int_init(int owner, uint64_t bit_width, auto val, size_t len0, size_t len1, size_t len2, size_t len3){
		auto Z = TG_int(bit_width, len0, len1, len2, len3);
		
		if(owner == PUBLIC)
			assign_vector(Z, val, bit_width, len0, len1, len2, len3);
		else
			register_input_vector(owner, bit_width, val, len0, len1, len2, len3);
		
		return Z;
	}

	/***clear memory alloacted to secret variable***/

	void clear_TG_int(auto& A){
		delete [] A;
	}
	void clear_TG_int(auto& A, uint64_t len0){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			delete [] A[i0];
		A.clear();
		A.shrink_to_fit();
	}	
	void clear_TG_int(auto& A, uint64_t len0, uint64_t len1){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				delete [] A[i0][i1];
		A.clear();
		A.shrink_to_fit();
	}
	void clear_TG_int(auto& A, uint64_t len0, uint64_t len1, uint64_t len2){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					delete [] A[i0][i1][i2];
		A.clear();
		A.shrink_to_fit();
	}
	void clear_TG_int(auto& A, uint64_t len0, uint64_t len1, uint64_t len2, uint64_t len3){
		for(uint64_t i0 = 0; i0 < len0; i0++)
			for(uint64_t i1 = 0; i1 < len1; i1++)
				for (uint64_t i2 = 0; i2 < len2; i2++)
					for (uint64_t i3 = 0; i3 < len3; i3++)
						delete [] A[i0][i1][i2][i3];
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
		
		labels_B = new block[cyc_rep_B*n1];
		labels_A = new block[cyc_rep_A*n2];
		
		twopc->gen_input_labels(n1, n1_0, labels_B, cycles_B, cyc_rep_B, n2, n2_0, labels_A, cycles_A, cyc_rep_A, IN);
		
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
		delete [] labels_A;
		delete [] labels_B;
		retreived_index_A = 0;
		retreived_index_B = 0;
	}
	
	void retrieve_input_labels(block*& retreived_label, int owner, uint64_t bit_width){
	/*Labels of input have to be retrieved in the same order they were registered with register_input(), must be called after gen_input_labels()*/
		if(owner == ALICE) 
			for(uint64_t i = 0; i < bit_width; i++)
				memcpy(&retreived_label[i], &labels_A[retreived_index_A++], sizeof(block));
		else 
			for(uint64_t i = 0; i < bit_width; i++)
				memcpy(&retreived_label[i], &labels_B[retreived_index_B++], sizeof(block));
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
	
	int64_t reveal(block* labels, uint64_t bit_width = 1, bool is_signed = true){
	/*Combine shares to reveal the secret value of an integer*/
		InputOutput* InOut = new InputOutput(0);
		InOut->init("", 0, bit_width, "", 0, 1);
		
		bool* o = new bool[bit_width];
		bool* o_hat = new bool[bit_width];
		bool* mask = new bool[bit_width];
		
		for(uint64_t i = 0; i < bit_width; i++)
			o_hat[i] = getLSB(labels[i]);
		
		if (party == ALICE){
			io->send_data(o_hat, bit_width); 		
			io->recv_data(mask, bit_width); 
		}
		else{		
			io->recv_data(mask, bit_width); 
			io->send_data(o_hat, bit_width); 
		}
		
		for(uint64_t i = 0; i < bit_width; i++)
			o[i] = twopc->logic_xor(o_hat[i], mask[i]);
		
		InOut->fill_output(o);
		string output_hex_str = InOut->read_output();
		
		vector<int64_t> output;	
		parseGCOutputString(output, output_hex_str, bit_width, 0, is_signed);
		
		delete InOut;
		delete[] o;
		delete[] o_hat;
		delete[] mask;
		return output.back();
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
	
	void fun(block*& y_x, block* a_x, block* b_x, uint64_t bit_width, string op){
		string netlist_address = string(NETLIST_PATH_PI) + op + "_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec_sh(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void fun(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b, string op){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		block* a1_x = new block[bit_width];
		block* b1_x = new block[bit_width];
		sign_extend(a1_x, a_x, bit_width, bit_width_a);
		sign_extend(b1_x, b_x, bit_width, bit_width_b);
		fun(y_x, a1_x, b1_x, bit_width, op);
		delete[] a1_x;
		delete[] b1_x;
	}
	void fun(block*& y_x, block* a_x, int64_t b, uint64_t bit_width, string op){
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		fun(y_x, a_x, b_x, bit_width, op);
		delete[] b_x;
	}
	
	/*y = a + b*/
	void add(block*& y_x, block* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "add");
	}	
	void add(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "add");
	}
	
	/*y = a - b*/
	void sub(block*& y_x, block* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "sub");
	}	
	void sub(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "sub");
	}
	
	/*y = a < b*/
	void lt(block*& y_x, block* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "lt");
	}	
	void lt(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "lt");
	}
	
	/*y = max(a, b)*/
	void max(block*& y_x, block* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "max");
	}	
	void max(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "max");
	}
	
	/*y = min(a, b)*/
	void min(block*& y_x, block* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "min");
	}	
	void min(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "min");
	}
	
	void logic(block*& y_x, block* a_x, block* b_x, uint64_t bit_width, string op){
		string netlist_address = string(NETLIST_PATH_PI) + op + "_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec_sh(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void logic(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b, string op){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		block* a1_x = new block[bit_width];
		block* b1_x = new block[bit_width];
		unsign_extend(a1_x, a_x, bit_width, bit_width_a);
		unsign_extend(b1_x, b_x, bit_width, bit_width_b);
		logic(y_x, a1_x, b1_x, bit_width, op);
		delete[] a1_x;
		delete[] b1_x;
	}
	
	/*y = a & b*/
	void and_(block*& y_x, block* a_x, block* b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "and");
	}	
	void and_(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "and");
	}
	void and_(block*& y_x, block* a_x, uint64_t b, uint64_t bit_width){
		bitset<64> bits(b);
		for (uint64_t i = 0; i < bit_width; i++)
			if (bits[i]) y_x[i] = a_x[i];
			else y_x[i] = twopc->label_const[0];
	}
	
	/*y = a | b*/
	void or_(block*& y_x, block* a_x, block* b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "or");
	}	
	void or_(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "or");
	}
	void or_(block*& y_x, block* a_x, uint64_t b, uint64_t bit_width){
		bitset<64> bits(b);
		for (uint64_t i = 0; i < bit_width; i++)
			if (bits[i]) y_x[i] = twopc->label_const[1];
			else y_x[i] = a_x[i];
	}
	
	/*y = a ^ b*/
	void xor_(block*& y_x, block* a_x, block* b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "xor");
	}	
	void xor_(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "xor");
	}
	void xor_(block*& y_x, block* a_x, uint64_t b, uint64_t bit_width){
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		xor_(y_x, a_x, b_x, bit_width);
		clear_TG_int(b_x);
	}
	
	/*y = -a*/
	void neg(block*& y_x, block* a_x, uint64_t bit_width){
		block* zero = TG_int_init(PUBLIC, bit_width, (int64_t)0);
		sub(y_x, zero, a_x, bit_width);
		clear_TG_int(zero);
	}

	/*y = ~a*/
	void not_(block*& y_x, block* a_x, uint64_t bit_width){
		block* one = new block[bit_width];
		for (uint64_t i = 0; i < bit_width; i++){
			one[i] = twopc->label_const[1];
		}
		xor_(y_x, one, a_x, bit_width);
		delete[] one;
	}
	
	/*a << shift*/
	void left_shift(block*& a_x, uint64_t shift, uint64_t bit_width){
		for (int64_t i = bit_width - 1; i >= (int64_t)shift; i--){
			a_x[i] = a_x[i - shift];
		}
		for (int64_t i = shift - 1; i >= 0; i--){
			a_x[i] = twopc->label_const[0];
		}
	}
	/*a >> shift*/
	void right_shift(block*& a_x, uint64_t shift, uint64_t bit_width){	
		for (uint64_t i = 0; i < bit_width - shift; i++){
			a_x[i] = a_x[i + shift];
		}
		for (uint64_t i = bit_width - shift; i < bit_width; i++){
			a_x[i] = a_x[bit_width - 1];
		}
	}
	
	/*y = a * b*/
	void mult(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		string netlist_address = string(NETLIST_PATH_PI) + "mult_" + to_string(bit_width_a) + "_" + to_string(bit_width_b) + "_" + to_string(bit_width_a+bit_width_b-1) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec_sh(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void mult(block*& y_x, block* a_x, block* b_x, uint64_t bit_width){
		mult(y_x, a_x, b_x, bit_width, bit_width);
	}
	void mult(block*& y_x, block* a_x, int64_t b, uint64_t bit_width_a, uint64_t bit_width_b){
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
		clear_TG_int(a1_x);
		clear_TG_int(a2_x);
	}
	
	void mat_mult(uint64_t row_A, uint64_t inner, uint64_t col_B, auto &A, auto &B, auto &C, int64_t rs_bits, uint64_t bit_width_A, uint64_t bit_width_B, uint64_t bit_width_C, uint64_t bit_width_G){
	
		string netlist_address = string(NETLIST_PATH_PI) + "mac_" + to_string(bit_width_A) + "_" + to_string(bit_width_B) + "_" + to_string(bit_width_G) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = inner, repeat = 1, output_mode = 3;
		
		block* labels_A = new block[row_A*inner*bit_width_A];
		block* labels_B = new block[inner*col_B*bit_width_B];
		block* labels_C = new block[bit_width_G];
		
		for (uint64_t i = 0; i < row_A; i++)
			for (uint64_t j = 0; j < inner; j++)
				for (uint64_t k = 0; k < bit_width_A; k++)	
					memcpy(labels_A + (i*inner + j)*bit_width_A + k, &A[i][j][k], sizeof(block));
			
		for (uint64_t i = 0; i < col_B; i++)
			for (uint64_t j = 0; j < inner; j++)
				for (uint64_t k = 0; k < bit_width_B; k++)	
					memcpy(labels_B + (i*inner + j)*bit_width_B + k, &B[j][i][k], sizeof(block));	
			
		for (uint64_t i = 0; i < row_A; i++){
			for (uint64_t j = 0; j < col_B; j++){
				sequential_2pc_exec_sh(twopc, labels_B+j*inner*bit_width_B, labels_A+i*inner*bit_width_A, nullptr, labels_C, party, io, &cf, cycles, repeat, output_mode);
				right_shift(labels_C, rs_bits, bit_width_G);
				assign(C[i][j], labels_C, bit_width_C);
				cout << fixed << setprecision(2) << setfill('0');
				cout << "\r   " << ((double)(i*col_B+j+1))/(row_A*col_B)*100 << "%";
			}
		}
		cout << endl;
		
		delete [] labels_A;
		delete [] labels_B;
		delete [] labels_C;
	}
	
	/*y = a / b*/
	void div(block*& y_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		string netlist_address = string(NETLIST_PATH_PI) + "div_" + to_string(bit_width_a) + "_" + to_string(bit_width_b) + "_" + to_string(bit_width_a) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec_sh(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void div(block*& y_x, block* a_x, block* b_x, uint64_t bit_width){
		div(y_x, a_x, b_x, bit_width, bit_width);
	}
	
	/*y = c? a : b*/
	void ifelse(block*& y_x, block* c_x, block* a_x, block* b_x, uint64_t bit_width){	
		block* ab_x = new block[2*bit_width];
		memcpy(ab_x, b_x, bit_width*sizeof(block));
		memcpy(ab_x + bit_width, a_x, bit_width*sizeof(block));		
		string netlist_address = string(NETLIST_PATH_PI) + "ifelse_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;	
		sequential_2pc_exec_sh(twopc, c_x, ab_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);		
		delete[] ab_x;
	}
	void ifelse(block*& y_x, block* c_x, block* a_x, block* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		block* a1_x = new block[bit_width];
		block* b1_x = new block[bit_width];
		sign_extend(a1_x, a_x, bit_width, bit_width_a);
		sign_extend(b1_x, b_x, bit_width, bit_width_b);
		ifelse(y_x, c_x, a1_x, b1_x, bit_width);
		delete[] a1_x;
		delete[] b1_x;
	}
	void ifelse(block*& y_x, block* c_x, block* a_x, int64_t b, uint64_t bit_width){	
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		block* ab_x = new block[2*bit_width];
		memcpy(ab_x, b_x, bit_width*sizeof(block));
		memcpy(ab_x + bit_width, a_x, bit_width*sizeof(block));		
		string netlist_address = string(NETLIST_PATH_PI) + "ifelse_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;	
		sequential_2pc_exec_sh(twopc, c_x, ab_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);		
		delete[] ab_x;
		clear_TG_int(b_x);
	}
	
	/*relu(a)*/		
	void relu(block* &a_x, uint64_t bit_width){
		block* sign_x = new block[1];
		not_(sign_x, &a_x[bit_width-1], 1);
		block* mask_x = new block[bit_width];
		for (uint64_t i = 0; i < bit_width-1; i++){
			mask_x[i] = sign_x[0];
		}
		mask_x[bit_width-1] = twopc->label_const[0];
		and_(a_x, a_x, mask_x, bit_width);
		delete[] sign_x;
		delete[] mask_x;
	}
	
};	

#endif //PROGRAM_INTERFACE_SH_H

#pragma GCC diagnostic pop