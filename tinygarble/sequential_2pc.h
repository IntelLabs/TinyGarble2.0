#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvla"

#ifndef SEQUENTIAL_C2PC_H__
#define SEQUENTIAL_C2PC_H__
#include "fpre.h"
#include <emp-tool/emp-tool.h>
#include <emp-ot/emp-ot.h>

//#define PRINT

class lmkvm{
	public:
	uint64_t num_bits;
	bool is_at;

	block* labels;
	block* mac;
	block* key;
	bool* value;
	uint8_t* mask;
	
	lmkvm(uint64_t num_bits){
		this->num_bits = num_bits;
		is_at = false;
		
		if(num_bits > 0){
			labels	= new block		[num_bits];
			mac		= new block		[num_bits];
			key		= new block		[num_bits];
			value	= new bool		[num_bits];
			mask	= new uint8_t	[num_bits];
		}
		else{
			labels	= nullptr;
			mac		= nullptr;
			key		= nullptr;
			value	= nullptr;
			mask	= nullptr;
		}
	}

	lmkvm* at (uint64_t index){
		lmkvm* R = new lmkvm(0);
		R->num_bits = num_bits - index;
		R->labels	= labels + index;
		R->mac		= mac + index;
		R->key		= key + index;
		R->value	= value + index;
		R->mask		= mask + index; 
		R->is_at	= true;
		return R;
	}

	lmkvm att (uint64_t index){
		lmkvm R(0);
		R.num_bits	= num_bits - index;
		R.labels	= labels + index;
		R.mac		= mac + index;
		R.key		= key + index;
		R.value		= value + index;
		R.mask		= mask + index; 
		R.is_at	= true;
		return R;
	}

	void copy(lmkvm* R, uint64_t index, uint64_t num_bits){
		memcpy(labels + index,	R->labels, num_bits*sizeof(block));
		memcpy(mac + index,		R->mac, num_bits*sizeof(block));
		memcpy(key + index,		R->key, num_bits*sizeof(block));
		memcpy(value + index,	R->value, num_bits*sizeof(bool));
		memcpy(mask + index,	R->mask, num_bits*sizeof(uint8_t));
	}
	void copy(lmkvm* R){
		copy(R, 0, num_bits);
	}

	void copy(lmkvm R, uint64_t index, uint64_t num_bits){
		memcpy(labels + index,	R.labels, num_bits*sizeof(block));
		memcpy(mac + index,		R.mac, num_bits*sizeof(block));
		memcpy(key + index,		R.key, num_bits*sizeof(block));
		memcpy(value + index,	R.value, num_bits*sizeof(bool));
		memcpy(mask + index,	R.mask, num_bits*sizeof(uint8_t));
	}
	void copy(lmkvm R){
		copy(R, 0, num_bits);
	}

	~lmkvm(){
		if (!is_at){
			delete[] labels;
			delete[] mac;
			delete[] key;
			delete[] value;
			delete[] mask;
		}
	}
};

class SequentialC2PC { public:
	NetIO * io;
	int party;
	
	const static int SSP = 5;//5*8 in fact...
	const block MASK = makeBlock(0x0ULL, 0xFFFFFULL);
	Fpre* fpre = nullptr;

	block *labels, *labels_1;
	block *mac, *mac_1;
	block *key, *key_1;
	bool *value, *value_1;
	uint8_t *mask_wire, *mask_wire_1;
	
	lmkvm* const_lmkvm;	
	
	int total_PRE, pre_ret; //index for retrieved preprocess mac, key, value
	block *preprocess_mac;
	block *preprocess_key;
	bool *preprocess_value;
	
	block *sigma_mac;
	block *sigma_key;
	bool *sigma_value;

	CircuitFile * cf;
	int num_inputs, num_ands;	
	
	bool *x1;
	bool *y1;
	bool *x2;
	bool *y2;

	PRG prg;
	PRP prp;
	
	block ***GT;
	block **GTK;
	block **GTM;
	bool  **GTv;
	
	int total_ANDS, ANDS_ret; //index for retrieved ANDS mac, key, value
	//not allocation
	block *ANDS_mac;
	block *ANDS_key;
	bool *ANDS_value;
	
	/*TinyGarble >>>*/
	int cycles, cyc_rep, output_mode, cid;
	/*<<< TinyGarble*/

	SequentialC2PC(NetIO * io, int party, int total_PRE, int total_ANDS) {
		this->party = party;
		this->io = io;
		this->total_PRE = total_PRE;
		this->total_ANDS = total_ANDS;
		
		ANDS_ret = 3*total_ANDS; //fpre->refill() should run the first time if there is at least one AND gate
		
		fpre = new Fpre(io, party, total_ANDS);

		preprocess_mac = new block[total_PRE];
		preprocess_key = new block[total_PRE];
		preprocess_value = new bool[total_PRE];
		
		const_lmkvm = new lmkvm(NUM_CONST);
	}
	~SequentialC2PC() {
		delete fpre;
		delete[] preprocess_mac;
		delete[] preprocess_key;
		delete[] preprocess_value;
		delete const_lmkvm;		
	}
	
	void function_independent() {
		io->flush();
		prg.random_bool(preprocess_value, total_PRE);
		if(fpre->party == ALICE) {
			fpre->abit1[0]->send(preprocess_key, total_PRE);
			fpre->io[0]->flush();
			fpre->abit2[0]->recv(preprocess_mac, preprocess_value, total_PRE);
			fpre->io2[0]->flush();
		} else {
			fpre->abit1[0]->recv(preprocess_mac, preprocess_value, total_PRE);
			fpre->io[0]->flush();
			fpre->abit2[0]->send(preprocess_key, total_PRE);
			fpre->io2[0]->flush();
		}
		pre_ret = 0;
		io->flush();
	}

	void new_const_labels(){	
		if(party == ALICE){
			prg.random_block(const_lmkvm->labels, NUM_CONST);
		}
		
		memcpy(const_lmkvm->mac, preprocess_mac, NUM_CONST*sizeof(block));
		memcpy(const_lmkvm->key, preprocess_key, NUM_CONST*sizeof(block));
		memcpy(const_lmkvm->value, preprocess_value, NUM_CONST*sizeof(bool));		
		pre_ret+= NUM_CONST;
		
		bool * mask = new bool[NUM_CONST];	
		memset(mask, false, NUM_CONST);
		block tmp;
		
		if(party == ALICE) {
			send_partial_block<SSP>(io, const_lmkvm->mac, NUM_CONST);
		} else {
			for(int i = 0; i < NUM_CONST; ++i) {
				recv_partial_block<SSP>(io, &tmp, 1);
				block ttt = xorBlocks(const_lmkvm->key[i], fpre->Delta);
				ttt =  _mm_and_si128(ttt, MASK);
				tmp =  _mm_and_si128(tmp, MASK);
				block mask_key = _mm_and_si128(const_lmkvm->key[i], MASK);
				if(block_cmp(&tmp, &mask_key, 1)) {
					mask[i] = false;
				} else if(block_cmp(&tmp, &ttt, 1)) {
					mask[i] = true;
				}
				else cout <<"no match! BOB\t"<<i<<endl;
			}
		}		
		
		if(party == ALICE) {
			io->recv_data(const_lmkvm->mask, NUM_CONST);
			for(int i = 0; i < NUM_CONST; ++i) {
				tmp = const_lmkvm->labels[i];
				if(const_lmkvm->mask[i]) tmp = xorBlocks(tmp, fpre->Delta);
				io->send_block(&tmp, 1);
			}
		} else {
			bool const_input [NUM_CONST] = {false, true};
			for(int i = 0; i < NUM_CONST; ++i) {
				const_lmkvm->mask[i] = logic_xor(const_input[i], const_lmkvm->value[i]);
				const_lmkvm->mask[i] = logic_xor(const_lmkvm->mask[i], mask[i]);
			}
			io->send_data(const_lmkvm->mask, NUM_CONST);
			io->recv_block(const_lmkvm->labels, NUM_CONST);
		}
		
		delete[] mask;
	}
	
	void new_input_labels(uint64_t n1, uint64_t n2, lmkvm* lmkvm_B, lmkvm* lmkvm_A, bool* input){
		int num_inputs = n1 + n2; //local to this function
		
		if(party == ALICE){
			prg.random_block(lmkvm_B->labels, n1);
			prg.random_block(lmkvm_A->labels, n2);
		}	

		uint64_t num_steps, step, last_step;

		num_steps = n1/total_PRE;
		step = min(n1, (uint64_t)total_PRE);
		last_step = n1%total_PRE;
		for(uint64_t i = 0; i < num_steps; ++i){
			function_independent();		
			memcpy(lmkvm_B->mac + i*step, preprocess_mac + pre_ret, step*sizeof(block));
			memcpy(lmkvm_B->key + i*step, preprocess_key + pre_ret, step*sizeof(block));
			memcpy(lmkvm_B->value + i*step, preprocess_value + pre_ret, step*sizeof(bool));	
		}
		function_independent();		
		memcpy(lmkvm_B->mac + num_steps*step, preprocess_mac + pre_ret, last_step*sizeof(block));
		memcpy(lmkvm_B->key + num_steps*step, preprocess_key + pre_ret, last_step*sizeof(block));
		memcpy(lmkvm_B->value + num_steps*step, preprocess_value + pre_ret, last_step*sizeof(bool));
		pre_ret+= last_step;

		num_steps = n2/total_PRE;
		step = min(n2, (uint64_t)total_PRE);
		last_step = n2%total_PRE;
		for(uint64_t i = 0; i < num_steps; ++i){
			function_independent();		
			memcpy(lmkvm_A->mac + i*step, preprocess_mac + pre_ret, step*sizeof(block));
			memcpy(lmkvm_A->key + i*step, preprocess_key + pre_ret, step*sizeof(block));
			memcpy(lmkvm_A->value + i*step, preprocess_value + pre_ret, step*sizeof(bool));	
		}
		function_independent();		
		memcpy(lmkvm_A->mac + num_steps*step, preprocess_mac + pre_ret, last_step*sizeof(block));
		memcpy(lmkvm_A->key + num_steps*step, preprocess_key + pre_ret, last_step*sizeof(block));
		memcpy(lmkvm_A->value + num_steps*step, preprocess_value + pre_ret, last_step*sizeof(bool));
		pre_ret+= last_step;		

		bool * mask_B = new bool[n1];
		bool * mask_A = new bool[n2];	
		memset(mask_B, false, n1);
		memset(mask_A, false, n2);

		block tmp;
		
		if(party == ALICE) {
			send_partial_block<SSP>(io, lmkvm_B->mac, n1);
			for(int i = 0; i < n2; ++i) {
				recv_partial_block<SSP>(io, &tmp, 1);
				block ttt = xorBlocks(lmkvm_A->key[i], fpre->Delta);
				ttt =  _mm_and_si128(ttt, MASK);
				block mask_key = _mm_and_si128(lmkvm_A->key[i], MASK);
				tmp =  _mm_and_si128(tmp, MASK);
				if(block_cmp(&tmp, &mask_key, 1))
					mask_A[i] = false;
				else if(block_cmp(&tmp, &ttt, 1))
					mask_A[i] = true;
				else cout <<"no match! ALICE\t"<<i<<endl;
			}
		} else {
			for(int i = 0; i < n1; ++i) {
				recv_partial_block<SSP>(io, &tmp, 1);
				block ttt = xorBlocks(lmkvm_B->key[i], fpre->Delta);
				ttt =  _mm_and_si128(ttt, MASK);
				tmp =  _mm_and_si128(tmp, MASK);
				block mask_key = _mm_and_si128(lmkvm_B->key[i], MASK);
				if(block_cmp(&tmp, &mask_key, 1)) {
					mask_B[i] = false;
				} else if(block_cmp(&tmp, &ttt, 1)) {
					mask_B[i] = true;
				}
				else cout <<"no match! BOB\t"<<i<<endl;
			}

			send_partial_block<SSP>(io, lmkvm_A->mac, n2);
		}		
		
		if(party == ALICE) {
			for(int i = 0; i < n2; ++i) {
				lmkvm_A->mask[i] = logic_xor(input[i], lmkvm_A->value[i]);
				lmkvm_A->mask[i] = logic_xor(lmkvm_A->mask[i], mask_A[i]);
			}
			io->recv_data(lmkvm_B->mask, n1);
			io->send_data(lmkvm_A->mask, n2);
			for(int i = 0; i < n1; ++i) {
				tmp = lmkvm_B->labels[i];
				if(lmkvm_B->mask[i]) tmp = xorBlocks(tmp, fpre->Delta);
				io->send_block(&tmp, 1);
			}
			for(int i = 0; i < n2; ++i) {
				tmp = lmkvm_A->labels[i];
				if(lmkvm_A->mask[i]) tmp = xorBlocks(tmp, fpre->Delta);
				io->send_block(&tmp, 1);
			}
		} else {
			for(int i = 0; i < n1; ++i) {
				lmkvm_B->mask[i] = logic_xor(input[i], lmkvm_B->value[i]);
				lmkvm_B->mask[i] = logic_xor(lmkvm_B->mask[i], mask_B[i]);
			}
			io->send_data(lmkvm_B->mask, n1);
			io->recv_data(lmkvm_A->mask, n2);
			io->recv_block(lmkvm_B->labels, n1);
			io->recv_block(lmkvm_A->labels, n2);
		}
		
		delete[] mask_B;
		delete[] mask_A;
	}

	void new_input_labels(uint64_t n1, uint64_t n1_0, uint64_t n2, uint64_t n2_0, int cycles, int cyc_rep, string input_hex_str, string init_hex_str, lmkvm* lmkvm_B, lmkvm* lmkvm_A){	
		int input_bit_width, init_bit_width;
		if (party == ALICE){
			input_bit_width = n2;
			init_bit_width = n2_0;
		}
		else{
			input_bit_width = n1;
			init_bit_width = n1_0;
		}	
		InputOutput* InOut = new InputOutput(0);	
		InOut->init(input_hex_str, input_bit_width, 0, init_hex_str, init_bit_width, cycles);	
		bool *in = new bool[input_bit_width];
		bool *IN = new bool[cyc_rep*input_bit_width];
			
		for(uint64_t cid = 0; cid < cyc_rep; ++cid){
			InOut->fill_input(in);
			memcpy(IN + cid*input_bit_width, in, input_bit_width*sizeof(bool));
		}	

		new_input_labels(cyc_rep*n1, cyc_rep*n2, lmkvm_B, lmkvm_A, IN);
		
		delete[] in;
		delete[] IN;
		delete InOut;
	}
	
	void init(CircuitFile * cf, int cycles, int cyc_rep, int output_mode){	
		this->cf = cf;	
		
		/*TinyGarble >>>*/
		this->cycles = cycles;
		this->cyc_rep = cyc_rep;
		this->output_mode = output_mode;
		/*<<< TinyGarble*/
		
		num_ands = 0;
		for(int i = 0; i < cf->num_gate; ++i) {
			if (cf->gates[4*i+3] == AND_GATE)
				++num_ands;
		}		
		num_inputs = cf->n0 + cf->n1 + cf->n2;
		
		x1 = new bool[num_ands];
		y1 = new bool[num_ands];
		x2 = new bool[num_ands];
		y2 = new bool[num_ands];
		
		labels = new block[cf->num_wire];
		labels_1 = new block[cf->num_wire];
		mac = new block[cf->num_wire];
		mac_1 = new block[cf->num_wire];
		key = new block[cf->num_wire];
		key_1 = new block[cf->num_wire];
		value = new bool[cf->num_wire];
		value_1 = new bool[cf->num_wire];
		mask_wire = new uint8_t[cf->num_wire];
		mask_wire_1 = new uint8_t[cf->num_wire];
		
		sigma_mac = new block[num_ands];
		sigma_key = new block[num_ands];
		sigma_value = new bool[num_ands];	
		
		GT = new block**[num_ands];
		GTK = new block*[num_ands];
		GTM = new block*[num_ands];
		GTv = new bool*[num_ands];

		for (int j = 0; j < num_ands; j++){
			GT[j] = new block*[4];
			for (int k = 0; k < 4; k++)
				GT[j][k] = new block[2];
			GTK[j] = new block[4];
			GTM[j] = new block[4];
			GTv[j] = new bool[4];
		}
		
		cid = 0;
	}
	
	void clear() {
		delete[] labels;
		delete[] labels_1;
		delete[] mac;
		delete[] mac_1;
		delete[] key;
		delete[] key_1;
		delete[] value;
		delete[] value_1;
		delete[] mask_wire;
		delete[] mask_wire_1;

		delete[] sigma_mac;
		delete[] sigma_key;
		delete[] sigma_value;

		delete[] x1;
		delete[] x2;
		delete[] y1;
		delete[] y2;
			
		delete[] GT;
		delete[] GTK;
		delete[] GTM;
		delete[] GTv;
	}	
	
	void copy_input_labels(lmkvm* lmkvm_B, lmkvm* lmkvm_A, lmkvm* lmkvm_S = nullptr){
			int offset = 0;

			memcpy(labels+offset, const_lmkvm->labels, NUM_CONST*sizeof(block));
			memcpy(mac+offset, const_lmkvm->mac, NUM_CONST*sizeof(block));
			memcpy(key+offset, const_lmkvm->key, NUM_CONST*sizeof(block));
			memcpy(value+offset, const_lmkvm->value, NUM_CONST*sizeof(bool));
			memcpy(mask_wire+offset, const_lmkvm->mask, NUM_CONST*sizeof(uint8_t));
			offset += NUM_CONST;

			if(cf->n0){
				memcpy(labels+offset, lmkvm_S->labels + cid*cf->n0, cf->n0*sizeof(block));
				memcpy(mac+offset, lmkvm_S->mac + cid*cf->n0, cf->n0*sizeof(block));
				memcpy(key+offset, lmkvm_S->key + cid*cf->n0, cf->n0*sizeof(block));
				memcpy(value+offset, lmkvm_S->value + cid*cf->n0, cf->n0*sizeof(bool));
				memcpy(mask_wire+offset, lmkvm_S->mask + cid*cf->n0, cf->n0*sizeof(uint8_t));
				offset += cf->n0;
			}

			if(cf->n1){
				memcpy(labels+offset, lmkvm_B->labels + cid*cf->n1, cf->n1*sizeof(block));
				memcpy(mac+offset, lmkvm_B->mac + cid*cf->n1, cf->n1*sizeof(block));
				memcpy(key+offset, lmkvm_B->key + cid*cf->n1, cf->n1*sizeof(block));
				memcpy(value+offset, lmkvm_B->value + cid*cf->n1, cf->n1*sizeof(bool));
				memcpy(mask_wire+offset, lmkvm_B->mask + cid*cf->n1, cf->n1*sizeof(uint8_t));
				offset += cf->n1;
			}

			if(cf->n2){
				memcpy(labels+offset, lmkvm_A->labels + cid*cf->n2, cf->n2*sizeof(block));
				memcpy(mac+offset, lmkvm_A->mac + cid*cf->n2, cf->n2*sizeof(block));
				memcpy(key+offset, lmkvm_A->key + cid*cf->n2, cf->n2*sizeof(block));
				memcpy(value+offset, lmkvm_A->value + cid*cf->n2, cf->n2*sizeof(bool));
				memcpy(mask_wire+offset, lmkvm_A->mask + cid*cf->n2, cf->n2*sizeof(uint8_t));
			}
	}
	
	void function_dependent_st() {
		int ands;
			
		if(party == ALICE) {
			prg.random_block(labels+NUM_CONST+num_inputs, cf->num_wire-(NUM_CONST+num_inputs));
		}
		
		if(ANDS_ret > 3*(total_ANDS-num_ands)){
			io->flush();
			fpre->refill();	
			ANDS_ret = 0;
		}

		ANDS_mac = fpre->MAC + ANDS_ret;
		ANDS_key = fpre->KEY + ANDS_ret;
		ANDS_value = fpre->r + ANDS_ret;
		ANDS_ret += 3*num_ands;
		
		for(int i = 0; i < cf->num_gate; ++i) {
			if (cf->gates[4*i+3] == AND_GATE) {
				if (pre_ret == total_PRE) function_independent();
				key[cf->gates[4*i+2]] = preprocess_key[pre_ret];
				mac[cf->gates[4*i+2]] = preprocess_mac[pre_ret];
				value[cf->gates[4*i+2]] = preprocess_value[pre_ret];	
				pre_ret++;				
			}
		}
		
		/*TinyGarble >>>*/
		for(int i = 0; i < cf->num_gate; ++i) {
			if (cf->gates[4*i+3] == DFF_GATE) {
				if((cid%cycles) == 0) {
					key[cf->gates[4*i+2]] = key[cf->gates[4*i+1]];
					mac[cf->gates[4*i+2]] = mac[cf->gates[4*i+1]];
					value[cf->gates[4*i+2]] = value[cf->gates[4*i+1]];
					if(party == ALICE)
						labels[cf->gates[4*i+2]] = labels[cf->gates[4*i+1]];
				}
				else {
					key[cf->gates[4*i+2]] = key_1[cf->gates[4*i]];
					mac[cf->gates[4*i+2]] = mac_1[cf->gates[4*i]];
					value[cf->gates[4*i+2]] = value_1[cf->gates[4*i]];
					if(party == ALICE)
						labels[cf->gates[4*i+2]] = labels_1[cf->gates[4*i]];
				}
				
			}
		}
		/*<<< TinyGarble*/
		for(int i = 0; i < cf->num_gate; ++i) {
			if (cf->gates[4*i+3] == XOR_GATE) {
				key[cf->gates[4*i+2]] = xorBlocks(key[cf->gates[4*i]], key[cf->gates[4*i+1]]);
				mac[cf->gates[4*i+2]] = xorBlocks(mac[cf->gates[4*i]], mac[cf->gates[4*i+1]]);
				value[cf->gates[4*i+2]] = logic_xor(value[cf->gates[4*i]], value[cf->gates[4*i+1]]);
				if(party == ALICE)
					labels[cf->gates[4*i+2]] = xorBlocks(labels[cf->gates[4*i]], labels[cf->gates[4*i+1]]);
			} else if (cf->gates[4*i+3] == NOT_GATE) {
				if(party == ALICE)
					labels[cf->gates[4*i+2]] = xorBlocks(labels[cf->gates[4*i]], fpre->Delta);
				value[cf->gates[4*i+2]] = value[cf->gates[4*i]];
				key[cf->gates[4*i+2]] = key[cf->gates[4*i]];
				mac[cf->gates[4*i+2]] = mac[cf->gates[4*i]];
			}				
		}
	
		ands = 0;
		for(int i = 0; i < cf->num_gate; ++i) {
			if (cf->gates[4*i+3] == AND_GATE) {
				x1[ands] = logic_xor(value[cf->gates[4*i]], ANDS_value[3*ands]);
				y1[ands] = logic_xor(value[cf->gates[4*i+1]], ANDS_value[3*ands+1]);	
				ands++;
			}
		}	

		if(party == ALICE) {
			send_bool(io, x1, num_ands);
			send_bool(io, y1, num_ands);
			recv_bool(io, x2, num_ands);
			recv_bool(io, y2, num_ands);
		} else {
			recv_bool(io, x2, num_ands);
			recv_bool(io, y2, num_ands);
			send_bool(io, x1, num_ands);
			send_bool(io, y1, num_ands);
		}

		for(int i = 0; i < num_ands; ++i) {
			x1[i] = logic_xor(x1[i], x2[i]); 
			y1[i] = logic_xor(y1[i], y2[i]); 
		}
		
		ands = 0;
		for(int i = 0; i < cf->num_gate; ++i) {
			if (cf->gates[4*i+3] == AND_GATE) {
				sigma_mac[ands] = ANDS_mac[3*ands+2];
				sigma_key[ands] = ANDS_key[3*ands+2];
				sigma_value[ands] = ANDS_value[3*ands+2];
				if(x1[ands]) {
					sigma_mac[ands] = xorBlocks(sigma_mac[ands], ANDS_mac[3*ands+1]);
					sigma_key[ands] = xorBlocks(sigma_key[ands], ANDS_key[3*ands+1]);
					sigma_value[ands] = logic_xor(sigma_value[ands], ANDS_value[3*ands+1]);
				}
				if(y1[ands]) {
					sigma_mac[ands] = xorBlocks(sigma_mac[ands], ANDS_mac[3*ands]);
					sigma_key[ands] = xorBlocks(sigma_key[ands], ANDS_key[3*ands]);
					sigma_value[ands] = logic_xor(sigma_value[ands], ANDS_value[3*ands]);
				}
				if(x1[ands] and y1[ands]) {
					if(party == ALICE)
						sigma_key[ands] = xorBlocks(sigma_key[ands], fpre->Delta);
					else
						sigma_value[ands] = not sigma_value[ands];
				}
#ifdef __debug
				block MM[] = {mac[cf->gates[4*i]], mac[cf->gates[4*i+1]], sigma_mac[ands]};
				block KK[] = {key[cf->gates[4*i]], key[cf->gates[4*i+1]], sigma_key[ands]};
				bool VV[] = {value[cf->gates[4*i]], value[cf->gates[4*i+1]], sigma_value[ands]};
				check(MM, KK, VV);
#endif
				ands++;
			}
		}//sigma_[] stores the and of input wires to each AND gates	

		block H[4][2];
		block K[4], M[4];
		bool r[4];
		ands = 0;
		for(int i = 0; i < cf->num_gate; ++i) {
#ifdef PRINT
			if(party == ALICE) {
				cout << "--------------------------------" << endl;
				cout << "gate " << i << ": " << cf->gates[4*i] << " " << cf->gates[4*i+1] << " " << cf->gates[4*i+2] << endl;
				cout << "Input labels:" << endl;
				printBlock(labels[cf->gates[4*i]]);
				printBlock(xorBlocks(labels[cf->gates[4*i]], fpre->Delta));
				printBlock(labels[cf->gates[4*i+1]]);
				printBlock(xorBlocks(labels[cf->gates[4*i+1]], fpre->Delta));
			}
#endif				
			if(cf->gates[4*i+3] == AND_GATE) {
				r[0] = logic_xor(sigma_value[ands] , value[cf->gates[4*i+2]]);
				r[1] = logic_xor(r[0] , value[cf->gates[4*i]]);
				r[2] = logic_xor(r[0] , value[cf->gates[4*i+1]]);
				r[3] = logic_xor(r[1] , value[cf->gates[4*i+1]]);
				if(party == BOB) r[3] = not r[3];

				M[0] = xorBlocks(sigma_mac[ands], mac[cf->gates[4*i+2]]);
				M[1] = xorBlocks(M[0], mac[cf->gates[4*i]]);
				M[2] = xorBlocks(M[0], mac[cf->gates[4*i+1]]);
				M[3] = xorBlocks(M[1], mac[cf->gates[4*i+1]]);

				K[0] = xorBlocks(sigma_key[ands], key[cf->gates[4*i+2]]);
				K[1] = xorBlocks(K[0], key[cf->gates[4*i]]);
				K[2] = xorBlocks(K[0], key[cf->gates[4*i+1]]);
				K[3] = xorBlocks(K[1], key[cf->gates[4*i+1]]);
				if(party == ALICE) K[3] = xorBlocks(K[3], fpre->Delta);

				if(party == ALICE) {
					Hash(H, labels[cf->gates[4*i]], labels[cf->gates[4*i+1]], i);
					for(int j = 0; j < 4; ++j) {
						H[j][0] = xorBlocks(H[j][0], M[j]);
						H[j][1] = xorBlocks(H[j][1], xorBlocks(K[j], labels[cf->gates[4*i+2]]));
						if(r[j]) 
							H[j][1] = xorBlocks(H[j][1], fpre->Delta);
#ifdef __debug
						check2(M[j], K[j], r[j]);
#endif
					}
#ifdef PRINT						
					cout << "r:" << endl;
					for(int j = 0; j < 4; ++j)
						cout << r[j];
					cout << endl;
#endif						
					for(int j = 0; j < 4; ++j ) {
						send_partial_block<SSP>(io, &H[j][0], 1);
						io->send_block(&H[j][1], 1);
					}

				} else {
					memcpy(GTK[ands], K, sizeof(block)*4);
					memcpy(GTM[ands], M, sizeof(block)*4);
					memcpy(GTv[ands], r, sizeof(bool)*4);
#ifdef __debug
					for(int j = 0; j < 4; ++j)
						check2(M[j], K[j], r[j]);
#endif
					for(int j = 0; j < 4; ++j ) {
						recv_partial_block<SSP>(io, &GT[ands][j][0], 1);
						io->recv_block(&GT[ands][j][1], 1);
					}

				}
				++ands;
			}
#ifdef PRINT				
			if(party == ALICE) {
				cout << "Output label:" << endl;
				printBlock(labels[cf->gates[4*i+2]]);
				printBlock(xorBlocks(labels[cf->gates[4*i+2]], fpre->Delta));
			}
#endif
		}
	}
	
	void evaluate(){
		int ands = 0;
		if(party == BOB) {
			for(int i = 0; i < cf->num_gate; ++i) {	
#ifdef PRINT						 
				cout << "--------------------------------" << endl;
				cout << "gate " << i << ": " << cf->gates[4*i] << " " << cf->gates[4*i+1] << " " << cf->gates[4*i+2] << endl;
				cout << "x_, y_: " << (int)mask_wire[cf->gates[4*i]] << ", " << (int)mask_wire[cf->gates[4*i+1]] << endl;
				cout << "input labels:" << endl;
				printBlock(labels[cf->gates[4*i]]);
				printBlock(labels[cf->gates[4*i+1]]);				
#endif				
				/*TinyGarble >>>*/
				if (cf->gates[4*i+3] == DFF_GATE) {
					if((cid%cycles) == 0){
						labels[cf->gates[4*i+2]] = labels[cf->gates[4*i+1]];
						mask_wire[cf->gates[4*i+2]] = mask_wire[cf->gates[4*i+1]];
					}
					else{
						labels[cf->gates[4*i+2]] = labels_1[cf->gates[4*i]];
						mask_wire[cf->gates[4*i+2]] = mask_wire_1[cf->gates[4*i]];
					} 
				}
				/*<<< TinyGarble*/
				else if (cf->gates[4*i+3] == XOR_GATE) {
					labels[cf->gates[4*i+2]] = xorBlocks(labels[cf->gates[4*i]], labels[cf->gates[4*i+1]]);
					mask_wire[cf->gates[4*i+2]] = logic_xor(mask_wire[cf->gates[4*i]], mask_wire[cf->gates[4*i+1]]);
				} else if (cf->gates[4*i+3] == AND_GATE) {
					int index = 2*mask_wire[cf->gates[4*i]] + mask_wire[cf->gates[4*i+1]];
					block H[2];
					Hash(H, labels[cf->gates[4*i]], labels[cf->gates[4*i+1]], i, index);
					GT[ands][index][0] = xorBlocks(GT[ands][index][0], H[0]);
					GT[ands][index][1] = xorBlocks(GT[ands][index][1], H[1]);

					block ttt = xorBlocks(GTK[ands][index], fpre->Delta);
					ttt =  _mm_and_si128(ttt, MASK);
					GTK[ands][index] =  _mm_and_si128(GTK[ands][index], MASK);
					GT[ands][index][0] =  _mm_and_si128(GT[ands][index][0], MASK);

					if(block_cmp(&GT[ands][index][0], &GTK[ands][index], 1))
						mask_wire[cf->gates[4*i+2]] = false;
					else if(block_cmp(&GT[ands][index][0], &ttt, 1))
						mask_wire[cf->gates[4*i+2]] = true;
					else 	cout <<ands <<"no match GT!"<<endl;			
#ifdef PRINT					
					cout << "Index: " << index << endl;
					cout << "Received r: " << (int)mask_wire[cf->gates[4*i+2]] << endl;
#endif					
					mask_wire[cf->gates[4*i+2]] = logic_xor(mask_wire[cf->gates[4*i+2]], GTv[ands][index]);

					labels[cf->gates[4*i+2]] = xorBlocks(GT[ands][index][1], GTM[ands][index]);
					
					ands++;
				} else {
					mask_wire[cf->gates[4*i+2]] = not mask_wire[cf->gates[4*i]];	
					labels[cf->gates[4*i+2]] = labels[cf->gates[4*i]];
				}
#ifdef PRINT				
				cout << "z_: " << (int)mask_wire[cf->gates[4*i+2]] << endl;
				cout << "output label: " << endl;
				printBlock(labels[cf->gates[4*i+2]]);
#endif				
			}
		}
		memcpy(labels_1, labels, (cf->num_wire)*sizeof(block));
		memcpy(mac_1, mac, (cf->num_wire)*sizeof(block));
		memcpy(key_1, key, (cf->num_wire)*sizeof(block));
		memcpy(value_1, value, (cf->num_wire)*sizeof(bool));
		memcpy(mask_wire_1, mask_wire, (cf->num_wire)*sizeof(uint8_t));
		
		cid++;	
	}
		
	void retrieve_shares(lmkvm* ret_lmkvm){		
		uint64_t output_start = cf->num_wire - cf->n3;
		
		memcpy(ret_lmkvm->labels, labels+output_start, (cf->n3)*sizeof(block));
		memcpy(ret_lmkvm->mac, mac+output_start, (cf->n3)*sizeof(block));
		memcpy(ret_lmkvm->key, key+output_start, (cf->n3)*sizeof(block));
		memcpy(ret_lmkvm->value, value+output_start, (cf->n3)*sizeof(bool));
		memcpy(ret_lmkvm->mask, mask_wire+output_start, (cf->n3)*sizeof(uint8_t));		
	}	
	
	void reveal(uint64_t n3, bool* output, lmkvm* ret_lmkvm){
		if(party == ALICE) {
			//send output mask data
			send_partial_block<SSP>(io, ret_lmkvm->mac, n3);
		}
		else {
			bool * o = new bool[n3];
			memset(o, false, n3);
			for(int i = 0; i < n3; ++i) {
				block tmp;
				recv_partial_block<SSP>(io, &tmp, 1);
				tmp =  _mm_and_si128(tmp, MASK);

				block ttt = xorBlocks(ret_lmkvm->key[i], fpre->Delta);
				ttt =  _mm_and_si128(ttt, MASK);
				ret_lmkvm->key[i] =  _mm_and_si128(ret_lmkvm->key[i], MASK);

				if(block_cmp(&tmp, &ret_lmkvm->key[i], 1))
					o[i] = false;
				else if(block_cmp(&tmp, &ttt, 1))
					o[i] = true;
				else 	cout <<"no match output label!"<<endl;
			}
			for(int i = 0; i < n3; ++i) {
				output[i] = logic_xor(o[i], ret_lmkvm->mask[i]);
				output[i] = logic_xor(output[i], ret_lmkvm->value[i]);
			}
			delete[] o;
		}
	}

	void Hash(block H[4][2], const block & a, const block & b, uint64_t i) {
		block A[2], B[2];
		A[0] = a; A[1] = xorBlocks(a, fpre->Delta);
		B[0] = b; B[1] = xorBlocks(b, fpre->Delta);
		A[0] = double_block(A[0]);
		A[1] = double_block(A[1]);
		B[0] = double_block(double_block(B[0]));
		B[1] = double_block(double_block(B[1]));

		H[0][1] = H[0][0] = xorBlocks(A[0], B[0]);
		H[1][1] = H[1][0] = xorBlocks(A[0], B[1]);
		H[2][1] = H[2][0] = xorBlocks(A[1], B[0]);
		H[3][1] = H[3][0] = xorBlocks(A[1], B[1]);
		for(uint64_t j = 0; j < 4; ++j) {
			H[j][0] = xorBlocks(H[j][0], _mm_set_epi64x(4*i+j, 0ULL));
			H[j][1] = xorBlocks(H[j][1], _mm_set_epi64x(4*i+j, 1ULL));
		}
		prp.permute_block((block *)H, 8);
	}

	void Hash(block H[2], block a, block b, uint64_t i, uint64_t row) {
		a = double_block(a);
		b = double_block(double_block(b));
		H[0] = H[1] = xorBlocks(a, b);
		H[0] = xorBlocks(H[0], _mm_set_epi64x(4*i+row, 0ULL));
		H[1] = xorBlocks(H[1], _mm_set_epi64x(4*i+row, 1ULL));
		prp.permute_block((block *)H, 2);
	}


	bool logic_xor(bool a, bool b) {
		return a!= b;
		if(a) if(not b) return true;
		if(not a) if(b) return true;
		return false;
	}
	string tostring(bool a) {
		if(a) return "T";
		else return "F";
	}
};

#endif// SEQUENTIAL_C2PC_H__

#pragma GCC diagnostic pop