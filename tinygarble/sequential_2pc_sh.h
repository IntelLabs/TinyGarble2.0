#ifndef SEQUENTIAL_C2PC_SH_H__
#define SEQUENTIAL_C2PC_SH_H__
#include <emp-tool/emp-tool.h>
#include <emp-ot/emp-ot.h>

//#define PRINT

class SequentialC2PC_SH { public:
	SHOTExtension<NetIO>* ote = nullptr;
	
	block Delta;
	block label_const[NUM_CONST];
	block* labels_b;
	
	block* labels;
	block* labels_1; //labels of the previous cycle
	
	block* labels_tr; //buffer for labels to be transferred 
	uint64_t tr_index = 0;

	CircuitFile * cf;
	NetIO * io;
	int num_ands; 
	int party;	

	PRG prg;
	PRP prp;
	
	block (*GT)[2];
	
	int cycles, cyc_rep, output_mode, cid; //TinyGarble >>>

	SequentialC2PC_SH(NetIO * io, int party) {
		this->party = party;
		this->io = io;
		
		ote = new SHOTExtension<NetIO>(io);
		
		if(party == ALICE) {
			prg.random_block(&Delta, 1);
			setLSB(Delta);
			prg.random_block(label_const, NUM_CONST);
			io->send_block(&label_const[0], 1);
			block tmp = xorBlocks(label_const[1], Delta);
			io->send_block(&tmp, 1);
		}
		else{
			memset(&Delta, 0, sizeof(block));
			io->recv_block(label_const, NUM_CONST);
		}
	}

	SequentialC2PC_SH(NetIO * io, int party, block Delta) {
		this->party = party;
		this->io = io;
		
		ote = new SHOTExtension<NetIO>(io);
		
		if(party == ALICE) {
			//memcpy(&(this->Delta), &Delta, sizeof(block));
			this->Delta = Delta;
			setLSB(Delta);
			prg.random_block(label_const, NUM_CONST);
			io->send_block(&label_const[0], 1);
			block tmp = xorBlocks(label_const[1], Delta);
			io->send_block(&tmp, 1);
		}
		else{
			io->recv_block(label_const, NUM_CONST);
		}
	}
	
	~SequentialC2PC_SH() {
		delete ote;
	}
	
	void init(int party, CircuitFile* cf, int cycles, int cyc_rep, int output_mode){
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

		labels_b = new block[cyc_rep*cf->n1];
		labels = new block[cf->num_wire];
		labels_1 = new block[cf->num_wire];
		
		if (output_mode == 2) labels_tr = new block[cyc_rep*cf->n3];
		else if (output_mode == 3) labels_tr = new block[(cyc_rep/cycles)*(cf->n3)];
		
		GT =  new block [num_ands][2];
		
		cid = 0;	
	}
	
	void clear() {

		delete[] GT;
		delete[] labels_b;
		
		delete [] labels;
		delete [] labels_1;
		
		if ((output_mode == 2) || (output_mode == 3)) delete[] labels_tr;
	}
	
	void gen_input_labels(uint64_t n1, uint64_t n1_0, block* &labels_B, uint64_t cycles_B, uint64_t cyc_rep_B, uint64_t n2, uint64_t n2_0, block* &labels_A, uint64_t cycles_A, uint64_t cyc_rep_A, bool *IN){
		
		block* label_init_b_  = new block[n1_0];
		block* label_input_b_ = new block[n1 - n1_0];
		
		if(party == ALICE){						
			for(uint64_t e = 0; e < cyc_rep_B; ++e){
				if(e%cycles_B == 0){ //generating labels for inits only at the first cycle
					prg.random_block(label_init_b_, n1_0 );
				}
				prg.random_block(label_input_b_, (n1 - n1_0));
				
				memcpy(labels_B + e*n1,			label_init_b_,	(n1_0)		*sizeof(block));
				memcpy(labels_B + e*n1 + n1_0,	label_input_b_,	(n1 - n1_0)	*sizeof(block));
			}
		}
		
		delete [] label_init_b_;
		delete [] label_input_b_;
			
		block* labels_B_OT = new block[cyc_rep_B*n1];		
		bool* mask_OT = new bool[cyc_rep_B*n1];
		
		if(party == ALICE){			
			block tmp;
			ote->send_cot(labels_B_OT, Delta, cyc_rep_B*n1); //labels_B_OT = K
			xorBlocks_arr(labels_B_OT, labels_B_OT, labels_B, cyc_rep_B*n1); //labels_B_OT = K+B
			
			io->recv_data(mask_OT, cyc_rep_B*n1); //mask_OT = r+b
			for(uint64_t i = 0; i < cyc_rep_B*n1; ++i){
				tmp = labels_B_OT[i];
				if(mask_OT[i]) tmp = xorBlocks(tmp, Delta); //tmp = K+B+(r+b)D
				io->send_block(&tmp, 1);
			}
		}	
		else{
			prg.random_bool(mask_OT, cyc_rep_B*n1); //mask_OT = r
			ote->recv_cot(labels_B_OT, mask_OT, cyc_rep_B*n1); //labels_B_OT = K+rD			
			
			for(uint64_t i = 0; i < cyc_rep_B*n1; ++i) 
				mask_OT[i] = logic_xor(mask_OT[i], IN[i]); //mask_OT = r+b			
			io->send_data(mask_OT, cyc_rep_B*n1); 
			io->recv_block(labels_B, cyc_rep_B*n1); //labels_B = K+B+(r+b)D
			xorBlocks_arr(labels_B, labels_B, labels_B_OT, cyc_rep_B*n1); //labels_B = K+B+(r+b)D + K+rD = B+bD
		}
		
		delete [] labels_B_OT;
		
		block* label_init_a_  = new block[n2_0];
		block* label_input_a_ = new block[n2 - n2_0];
		
		if(party == ALICE){						
			for(uint64_t e = 0; e < cyc_rep_A; ++e){
				if(e%cycles_A == 0){ //generating labels for inits only at the first cycle
					prg.random_block(label_init_a_, n2_0 );
				}
				prg.random_block(label_input_a_, (n2 - n2_0));
				
				memcpy(labels_A + e*n2,			label_init_a_,	(n2_0)		*sizeof(block));
				memcpy(labels_A + e*n2 + n2_0,	label_input_a_,	(n2 - n2_0)	*sizeof(block));
			}
		}
		
		if(party == ALICE){			
			block tmp;				
			for(uint64_t i = 0; i < cyc_rep_A*n2; ++i){
				tmp = labels_A[i];
				if(IN[i]) tmp = xorBlocks(tmp, Delta);
				io->send_block(&tmp, 1);
			}
		}	
		else{			
			io->recv_block(labels_A, cyc_rep_A*n2);
		}
		
		delete [] label_init_a_;
		delete [] label_input_a_;
		delete [] mask_OT;
	}
	
	void gen_input_labels(uint64_t n1, uint64_t n1_0, block* &labels_B, uint64_t cycles_B, uint64_t cyc_rep_B, uint64_t n2, uint64_t n2_0, block* &labels_A, uint64_t cycles_A, uint64_t cyc_rep_A, string input_hex_str, string init_hex_str){
		
		uint64_t cycles_, cyc_rep_, input_bit_width, init_bit_width, output_bit_width = 0;
		if (party == ALICE){
			cycles_ = cycles_A;
			cyc_rep_ = cyc_rep_A;
			input_bit_width	= n2;
			init_bit_width	= n2_0;
		}
		else{
			cycles_ = cycles_B;
			cyc_rep_ = cyc_rep_B;
			input_bit_width	= n1;
			init_bit_width	= n1_0;
		}	
		InputOutput* InOut = new InputOutput(0);
		InOut->init(input_hex_str, input_bit_width, output_bit_width, init_hex_str, init_bit_width, cycles_);
		
		bool *in = new bool[input_bit_width];
		bool *IN = new bool[cyc_rep_*input_bit_width];
		
		for(uint64_t e = 0; e < cyc_rep_; ++e){
			InOut->fill_input(in);
			memcpy(IN + e*input_bit_width, in, input_bit_width*sizeof(bool));
		}
		
		gen_input_labels(n1, n1_0, labels_B, cycles_B, cyc_rep_B, n2, n2_0, labels_A, cycles_A, cyc_rep_A, IN);	

		delete InOut;
		delete [] in;
		delete [] IN;
	}
	
	void copy_input_labels(block* labels_B, block* labels_A, block* labels_S = nullptr){	
			memcpy(labels,			 							label_const							, NUM_CONST				*sizeof(block));	
		if(cf->n0){
			memcpy(labels + NUM_CONST, 		 					labels_S + cid*cf->n0				, cf->n0_0				*sizeof(block));
			memcpy(labels + NUM_CONST + cf->n0_0,				labels_S + cid*cf->n0 + cf->n0_0	, (cf->n0 - cf->n0_0)	*sizeof(block));
		}
			memcpy(labels + NUM_CONST + cf->n0, 				labels_B + cid*cf->n1				, cf->n1_0				*sizeof(block));
			memcpy(labels + NUM_CONST + cf->n0+cf->n1_0,		labels_B + cid*cf->n1 + cf->n1_0	, (cf->n1 - cf->n1_0)	*sizeof(block));
			memcpy(labels + NUM_CONST + cf->n0+cf->n1,			labels_A + cid*cf->n2				, cf->n2_0				*sizeof(block));
			memcpy(labels + NUM_CONST + cf->n0+cf->n1+cf->n2_0, labels_A + cid*cf->n2 + cf->n2_0	, (cf->n2 - cf->n2_0)	*sizeof(block));
	}
	
	void garble() {			
		if(party == ALICE) {
			int ands = 0;
			for(int i = 0; i < cf->num_gate; ++i) {
#ifdef PRINT
				cout << "--------------------------------" << endl;
				cout << "gate " << i << ": " << cf->gates[4*i];
				if((cf->gates[4*i+3] != NOT_GATE)) cout << " " << cf->gates[4*i+1];
				cout << " " << cf->gates[4*i+2] << " ";
				if (cf->gates[4*i+3] == DFF_GATE) cout << "DFF" << endl;
				else if (cf->gates[4*i+3] == XOR_GATE) cout << "XOR" << endl;
				else if (cf->gates[4*i+3] == NOT_GATE) cout << "NOT" << endl;
				else if (cf->gates[4*i+3] == AND_GATE) cout << "AND" << endl;
				cout << "Input labels:" << endl;
				printBlock(labels[cf->gates[4*i]]);
				printBlock(xorBlocks(labels[cf->gates[4*i]], Delta));
				if((cf->gates[4*i+3] != NOT_GATE)){
					printBlock(labels[cf->gates[4*i+1]]);
					printBlock(xorBlocks(labels[cf->gates[4*i+1]], Delta));
				}
#endif
				/*TinyGarble >>>*/
				if (cf->gates[4*i+3] == DFF_GATE) {
					if((cid%cycles) == 0) 
						labels[cf->gates[4*i+2]] = labels[cf->gates[4*i+1]];
					else {
						labels[cf->gates[4*i+2]] = labels_1[cf->gates[4*i]];
					}						
				}
				/*<<< TinyGarble*/					
				else if (cf->gates[4*i+3] == XOR_GATE) 
					labels[cf->gates[4*i+2]] = xorBlocks(labels[cf->gates[4*i]], labels[cf->gates[4*i+1]]);
				else if (cf->gates[4*i+3] == NOT_GATE)
					labels[cf->gates[4*i+2]] = xorBlocks(labels[cf->gates[4*i]], Delta);				
				/* TwoHalvesMakeAWhole: https://eprint.iacr.org/2014/756.pdf */
				else if (cf->gates[4*i+3] == AND_GATE) {
					block Wa0 = labels[cf->gates[4*i]], Wb0 = labels[cf->gates[4*i+1]];
					block Wa1 = xorBlocks(Wa0, Delta), Wb1 = xorBlocks(Wb0, Delta); 
					uint8_t mask_a = (uint8_t)getLSB(Wa0), mask_b = (uint8_t)getLSB(Wb0); //pa, pb
					uint64_t jG = 2*ands, jE = 2*ands + 1; 
					
					block TG, TG0, TG1, WG;
					Hash(TG0, Wa0, jG);  
					Hash(TG1, Wa1, jG); 
					TG = xorBlocks(TG0, TG1);
					if(mask_b) TG = xorBlocks(TG, Delta);
					WG = TG0;
					if(mask_a) WG = xorBlocks(WG, TG);
					
					block TE, TE0, TE1, WE;
					Hash(TE0, Wb0, jE);  
					Hash(TE1, Wb1, jE); 
					TE = xorBlocks(TE0, TE1);
					TE = xorBlocks(TE, Wa0);
					WE = TE0;
					if(mask_b) WE = xorBlocks(WE, xorBlocks(TE, Wa0));
					
					GT[ands][0] = TG;
					GT[ands][1] = TE;
					labels[cf->gates[4*i+2]] = xorBlocks(WG, WE);
#ifdef PRINT						
					cout << "GT:" << endl;
					printBlock(GT[ands][0]);
					printBlock(GT[ands][1]);
#endif							
					++ands;
				}
#ifdef PRINT
				cout << "Output label:" << endl;
				printBlock(labels[cf->gates[4*i+2]]);
				printBlock(xorBlocks(labels[cf->gates[4*i+2]], Delta));
				cout << "lambdas: " << getLSB(labels[cf->gates[4*i]]);
				if((cf->gates[4*i+3] != NOT_GATE)) cout << ", " << getLSB(labels[cf->gates[4*i+1]]);
				cout << ", " << getLSB(labels[cf->gates[4*i+2]]) << endl;
#endif
			}/*i*/
			io->send_block(GT[0], 2*num_ands); //GRR3
		}/*party*/		
		else {				
			io->recv_block(GT[0], 2*num_ands); //GRR3	
		}
	}
	
	void evaluate (/*bool * input, */bool * output) {		
		bool * o = new bool[cf->n3];
		memset(o, false, cf->n3);
		
		if(party == ALICE) {	
			//send output mask data
			if((output_mode == 0) || ((output_mode == 1)&&(((cid+1)%cycles) == 0))){ //TinyGarble >>> 
				for(int i = 0; i < cf->n3; ++i)
					o[i] = getLSB(labels[cf->num_wire - cf->n3 + i]);
				io->send_data(o, cf->n3); 
			}
		} 
		else {
			if((output_mode == 0) || ((output_mode == 1)&&(((cid+1)%cycles) == 0))){ //TinyGarble >>>
				io->recv_data(o, cf->n3); //recv_bool(io, o, cf->n3): does not work for cf->n3 >= 8 on sum_8bit_1cc
			}								
						
			int ands = 0;
			
			for(int i = 0; i < cf->num_gate; ++i) {	
#ifdef PRINT						 
				cout << "--------------------------------" << endl;
				cout << "gate " << i << ": " << cf->gates[4*i] << " " << cf->gates[4*i+1] << " " << cf->gates[4*i+2] << " ";
				if (cf->gates[4*i+3] == DFF_GATE) cout << "DFF" << endl;
				else if (cf->gates[4*i+3] == XOR_GATE) cout << "XOR" << endl;
				else if (cf->gates[4*i+3] == NOT_GATE) cout << "NOT" << endl;
				else if (cf->gates[4*i+3] == AND_GATE) cout << "AND" << endl;
				cout << "input labels:" << endl;
				printBlock(labels[cf->gates[4*i]]);
				if((cf->gates[4*i+3] != NOT_GATE)) printBlock(labels[cf->gates[4*i+1]]);
#endif				
				/*TinyGarble >>>*/
				if (cf->gates[4*i+3] == DFF_GATE) {
					if((cid%cycles) == 0){
						labels[cf->gates[4*i+2]] = labels[cf->gates[4*i+1]];
					}
					else{
						labels[cf->gates[4*i+2]] = labels_1[cf->gates[4*i]];
					} 
				}
				/*<<< TinyGarble*/
				else if (cf->gates[4*i+3] == XOR_GATE) {
					labels[cf->gates[4*i+2]] = xorBlocks(labels[cf->gates[4*i]], labels[cf->gates[4*i+1]]);
				}  
				else if (cf->gates[4*i+3] == NOT_GATE) {	
					labels[cf->gates[4*i+2]] = labels[cf->gates[4*i]];
				}				
				/* TwoHalvesMakeAWhole: https://eprint.iacr.org/2014/756.pdf */
				else if (cf->gates[4*i+3] == AND_GATE) {
					block Wa = labels[cf->gates[4*i]], Wb = labels[cf->gates[4*i+1]];
					uint8_t mask_a = (uint8_t)getLSB(Wa), mask_b = (uint8_t)getLSB(Wb); //sa, sb
					uint64_t jG = 2*ands, jE = 2*ands + 1; 
					
					block TG = GT[ands][0], TE = GT[ands][1];
					block WG, WE;
					Hash(WG, Wa, jG);
					if(mask_a) WG = xorBlocks(WG, TG);
					Hash(WE, Wb, jE);
					if(mask_b) WE = xorBlocks(WE, xorBlocks(TE, Wa));
					
					labels[cf->gates[4*i+2]] = xorBlocks(WG, WE);
#ifdef PRINT					
					cout << "GT:" << endl;
					printBlock(GT[ands][0]);
					printBlock(GT[ands][1]);
#endif									
					ands++;
				}
#ifdef PRINT	
				cout << "output label: " << endl;
				printBlock(labels[cf->gates[4*i+2]]);
				cout << "hats: " << (int)getLSB(labels[cf->gates[4*i]]) << ", ";
				if((cf->gates[4*i+3] != NOT_GATE)) cout << (int)getLSB(labels[cf->gates[4*i+1]])<< ", ";		
				cout << (int)getLSB(labels[cf->gates[4*i+2]]) << endl;
#endif				
			}
			
			if((output_mode == 0) || ((output_mode == 1)&&(((cid+1)%cycles) == 0))){ //TinyGarble >>>
				for(int i = 0; i < cf->n3; ++i) {
					bool o_hat = getLSB(labels[cf->num_wire - cf->n3 + i]);
					output[i] = logic_xor(o[i], o_hat);
				}
			}		
		}
		
		memcpy(labels_1, labels, (cf->num_wire)*sizeof(block));
		
		if ((output_mode == 2) || ((output_mode == 3)&&(((cid+1)%cycles) == 0))){
			memcpy(labels_tr + tr_index, labels + (cf->num_wire - cf->n3), (cf->n3)*sizeof(block));
			tr_index += cf->n3;
		}
		
		cid++;		
			
		delete[] o;
	}
	
	void retrieve_shares(block*& labels_R){		
		if (output_mode == 2) {
			memcpy(labels_R, labels_tr, cyc_rep*(cf->n3)*sizeof(block));
			tr_index -= cyc_rep*(cf->n3);
		}
		else if (output_mode == 3){
			memcpy(labels_R, labels_tr, (cyc_rep/cycles)*(cf->n3)*sizeof(block));
			tr_index -= (cyc_rep/cycles)*(cf->n3);
		}
	}

	void Hash(block H[4], const block & a, const block & b, uint64_t i, int index[4]) {
		block A[2], B[2];
		A[0] = a; A[1] = xorBlocks(a, Delta);
		B[0] = b; B[1] = xorBlocks(b, Delta);
		A[0] = double_block(A[0]);
		A[1] = double_block(A[1]);
		B[0] = double_block(double_block(B[0]));
		B[1] = double_block(double_block(B[1]));

		H[0] = xorBlocks(A[0], B[0]);
		H[1] = xorBlocks(A[0], B[1]);
		H[2] = xorBlocks(A[1], B[0]);
		H[3] = xorBlocks(A[1], B[1]);
		
		for(uint64_t j = 0; j < 4; ++j) {
			H[j] = xorBlocks(H[j], _mm_set_epi64x(4*i+index[j], 0ULL));
		}
		prp.permute_block((block *)H, 4);
	}

	void Hash(block &H, block a, block b, uint64_t i, uint64_t row) {
		a = double_block(a);
		b = double_block(double_block(b));
		H = xorBlocks(a, b);
		H = xorBlocks(H, _mm_set_epi64x(4*i+row, 0ULL));
		prp.permute_block(&H, 1);
	}

	void Hash(block &H, block a, uint64_t i) {
		a = double_block(a);
		H = xorBlocks(a, _mm_set_epi64x(4*i, 0ULL));
		prp.permute_block(&H, 1);
	}

	bool logic_xor(bool a, bool b) {
		return a!= b;
		if(a) if(not b) return true;
		if(not a) if(b) return true;
		return false;
	}	
	
	void compute_garbled_index(int index[4], uint8_t mask_a, uint8_t mask_b){
		index [0] = 2*mask_a     + mask_b    ;
		index [1] = 2*mask_a     + (1-mask_b);
		index [2] = 2*(1-mask_a) + mask_b    ;
		index [3] = 2*(1-mask_a) + (1-mask_b);
	}
	
	string tostring(bool a) {
		if(a) return "T";
		else return "F";
	}
};
#endif// SEQUENTIAL_C2PC_SH_H__
