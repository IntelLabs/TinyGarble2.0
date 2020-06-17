#ifndef PROGRAM_INTERFACE_EXEC_H
#define PROGRAM_INTERFACE_EXEC_H

#include "tinygarble/program_interface.h"
#include "exec/exec_common.h"

void unit_test(auto TGPI) {

	cout << "----- test phase 0: scalers ----" << endl;

	uint64_t a_bits = 19, b_bits = 24, m_bits = MAX(a_bits, b_bits);
	int64_t a = rand_L_U(), b = rand_L_U();
	cout << "testing with a = " << a << ", b = " << b << endl; 
	
	auto a_x = TGPI->TG_int_init(ALICE, a_bits, a);
	auto b_x = TGPI->TG_int_init(BOB, b_bits, b);
		
	/*needed for test phase 2: vectors, but has to be done before gen_input_labels() ---->*/	
	uint64_t vector_bits = 64;	
	auto A0 = make_vector<int64_t>(8);	
	input_vector(A0, 8);
	auto A1 = make_vector<int64_t>(4, 3);		
	input_vector(A1, 4, 3);
	auto A2 = make_vector<int64_t>(2, 3, 4, 5);	
	input_vector(A2, 2, 3, 4, 5);		
	auto B0 = make_vector<int64_t>(5, 5, 1, 16);
	input_vector(B0, 5, 5, 1, 16);
	auto B1 = make_vector<int64_t>(3, 5);	
	input_vector(B1, 3, 5);
	auto B2 = make_vector<int64_t>(4);
	input_vector(B2, 4);	
	
	auto A0_x = TGPI->TG_int_init(ALICE, vector_bits, A0, 8);
	auto A1_x = TGPI->TG_int_init(ALICE, vector_bits, A1, 4, 3);
	auto A2_x = TGPI->TG_int_init(ALICE, vector_bits, A2, 2, 3, 4, 5);
	auto B0_x = TGPI->TG_int_init(BOB, vector_bits, B0, 5, 5, 1, 16);
	auto B1_x = TGPI->TG_int_init(BOB, vector_bits, B1, 3, 5);
	auto B2_x = TGPI->TG_int_init(BOB, vector_bits, B2, 4);
	
	TGPI->gen_input_labels();
	
	TGPI->retrieve_input_labels(a_x, ALICE, a_bits);
	TGPI->retrieve_input_labels(b_x, BOB, b_bits);
	int64_t a_chk = TGPI->reveal(a_x, a_bits);
	int64_t b_chk = TGPI->reveal(b_x, b_bits);
	if (TGPI->party == BOB) 
		verify_n_report("register inputs, retrieve labels and reveal", (vector<int64_t>){a_chk, b_chk}, (vector<int64_t>){a, b});
	
	auto t0_x = TGPI->TG_int_init(PUBLIC, a_bits, a);
	auto t1_x = TGPI->TG_int(a_bits);
	TGPI->assign(t1_x, b_x, a_bits, b_bits);
	int64_t t0_chk = TGPI->reveal(t0_x, a_bits);	
	int64_t t1_chk = TGPI->reveal(t1_x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("assign public or secret values", (vector<int64_t>){t0_chk, t1_chk}, (vector<int64_t>){a, b});

	auto t2_x = TGPI->TG_int(m_bits);
	auto t2__x = TGPI->TG_int(a_bits);
	TGPI->add(t2_x, a_x, b_x, a_bits, b_bits);
	TGPI->add(t2__x, a_x, b, a_bits);
	int64_t t2_chk = TGPI->reveal(t2_x, m_bits);
	int64_t t2__chk = TGPI->reveal(t2__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("addition of public or secret values", (vector<int64_t>){t2_chk, t2__chk}, (vector<int64_t>){a + b, a + b});

	auto t3_x = TGPI->TG_int_init(PUBLIC, m_bits, (int64_t)0);
	TGPI->add(t3_x, t3_x, a_x, a_bits);
	TGPI->add(t3_x, t3_x, b_x, a_bits, b_bits);
	TGPI->add(t3_x, t3_x, t0_x, a_bits);
	TGPI->add(t3_x, t3_x, t1_x, a_bits);
	TGPI->add(t3_x, t3_x, t2_x, a_bits);
	int64_t t3_chk = TGPI->reveal(t3_x, m_bits);
	if (TGPI->party == BOB)
		verify_n_report("accumulation", t3_chk, a + b + t0_chk + t1_chk + t2_chk);

	auto t4_x = TGPI->TG_int(m_bits);
	auto t4__x = TGPI->TG_int(a_bits);
	TGPI->sub(t4_x, a_x, b_x, a_bits, b_bits);
	TGPI->sub(t4__x, a_x, b, a_bits);
	int64_t t4_chk = TGPI->reveal(t4_x, m_bits);
	int64_t t4__chk = TGPI->reveal(t4__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("subtraction of public or secret values", (vector<int64_t>){t4_chk, t4__chk}, (vector<int64_t>){a - b, a - b});

	auto t5_x = TGPI->TG_int(1);
	auto t5__x = TGPI->TG_int(1);
	TGPI->lt(t5_x, a_x, b_x, a_bits, b_bits);
	TGPI->lt(t5__x, a_x, b, a_bits);
	int64_t t5_chk = TGPI->reveal(t5_x, 1, false);
	int64_t t5__chk = TGPI->reveal(t5__x, 1, false);
	if (TGPI->party == BOB) 
		verify_n_report("comparison of public or secret values", (vector<int64_t>){t5_chk, t5__chk}, (vector<int64_t>){(int64_t)(a < b), (int64_t)(a < b)});

	auto t6_x = TGPI->TG_int(m_bits);
	auto t6__x = TGPI->TG_int(a_bits);
	TGPI->max(t6_x, a_x, b_x, a_bits, b_bits);
	TGPI->max(t6__x, a_x, b, a_bits);
	int64_t t6_chk = TGPI->reveal(t6_x, m_bits);
	int64_t t6__chk = TGPI->reveal(t6__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("maximum of public or secret values", (vector<int64_t>){t6_chk, t6__chk}, (vector<int64_t>){MAX(a, b), MAX(a, b)});

	auto t7_x = TGPI->TG_int(m_bits);
	auto t7__x = TGPI->TG_int(a_bits);
	TGPI->min(t7_x, a_x, b_x, a_bits, b_bits);
	TGPI->min(t7__x, a_x, b, a_bits);
	int64_t t7_chk = TGPI->reveal(t7_x, m_bits);
	int64_t t7__chk = TGPI->reveal(t7__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("minimum of public or secret values", (vector<int64_t>){t7_chk, t7__chk}, (vector<int64_t>){MIN(a, b), MIN(a, b)});

	auto t8_x = TGPI->TG_int(a_bits);
	TGPI->neg(t8_x, a_x, a_bits);
	int64_t t8_chk = TGPI->reveal(t8_x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("negate", t8_chk, -a);

	auto t9_x = TGPI->TG_int(a_bits + b_bits - 1);
	auto t9__x = TGPI->TG_int(a_bits + b_bits - 1);
	TGPI->mult(t9_x, a_x, b_x, a_bits, b_bits);
	TGPI->mult(t9__x, a_x, b, a_bits, b_bits);
	int64_t t9_chk = TGPI->reveal(t9_x, a_bits + b_bits - 1);
	int64_t t9__chk = TGPI->reveal(t9__x, a_bits + b_bits - 1);
	if (TGPI->party == BOB) 
		verify_n_report("multiplication of public or secret values", (vector<int64_t>){t9_chk, t9__chk}, (vector<int64_t>){a * b, a * b});

	auto t10_x = TGPI->TG_int(a_bits);
	TGPI->div(t10_x, a_x, b_x, a_bits, b_bits);
	int64_t t10_chk = TGPI->reveal(t10_x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("division", t10_chk, a/b);

	auto t11_x = TGPI->TG_int(m_bits);
	auto t11__x = TGPI->TG_int(a_bits);
	TGPI->ifelse(t11_x, t5_x, a_x, b_x, a_bits, b_bits);
	TGPI->ifelse(t11__x, t5_x, a_x, b, a_bits);
	int64_t t11_chk = TGPI->reveal(t11_x, m_bits);
	int64_t t11__chk = TGPI->reveal(t11__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("if else public or secret values", (vector<int64_t>){t11_chk, t11__chk}, (vector<int64_t>){MIN(a, b), MIN(a, b)});

	int64_t t12 = rand_L_U(), t13 = rand_L_U();
	cout << "testing shift with a = " << t12 << ", b = " << t13 << endl; 
	int64_t t12_ref = t12 << 3, t13_ref = t13 >> 3;
	auto t12_x = TGPI->TG_int_init(PUBLIC, m_bits, t12);
	auto t13_x = TGPI->TG_int_init(PUBLIC, m_bits, t13);
	TGPI->left_shift(t12_x, 3, m_bits);
	TGPI->right_shift(t13_x, 3, m_bits);
	int64_t t12_chk = TGPI->reveal(t12_x, m_bits);
	int64_t t13_chk = TGPI->reveal(t13_x, m_bits);
	if (TGPI->party == BOB) 
		verify_n_report("left and right shift", (vector<int64_t>){t12_chk, t13_chk}, (vector<int64_t>){t12_ref, t13_ref});

	uint64_t t14 = urand_U(), t15 = urand_U();
	auto t14_x = TGPI->TG_int_init(PUBLIC, a_bits, t14);
	auto t15_x = TGPI->TG_int_init(PUBLIC, b_bits, t15);
	cout << "testing logical op with a = " << t14 << ", b = " << t15 << endl; 

    auto t16_x = TGPI->TG_int(m_bits);
	auto t16__x = TGPI->TG_int(a_bits);
	TGPI->and_(t16_x, t14_x, t15_x, a_bits, b_bits);
	TGPI->and_(t16__x, t14_x, t15, a_bits);
	uint64_t t16_chk = TGPI->reveal(t16_x, m_bits, false);
	uint64_t t16__chk = TGPI->reveal(t16__x, a_bits, false);
	if (TGPI->party == BOB) 
		verify_n_report("logical and of public or secret values", (vector<uint64_t>){t16_chk, t16__chk}, (vector<uint64_t>){t14 & t15, t14 & t15});

	auto t17_x = TGPI->TG_int(m_bits);
	auto t17__x = TGPI->TG_int(a_bits);
	TGPI->or_(t17_x, t14_x, t15_x, a_bits, b_bits);
	TGPI->or_(t17__x, t14_x, t15, a_bits);
	uint64_t t17_chk = TGPI->reveal(t17_x, m_bits, false);
	uint64_t t17__chk = TGPI->reveal(t17__x, a_bits, false);
	if (TGPI->party == BOB) 
		verify_n_report("logical or of public or secret values", (vector<uint64_t>){t17_chk, t17__chk}, (vector<uint64_t>){t14 | t15, t14 | t15});

	auto t18_x = TGPI->TG_int(m_bits);
	auto t18__x = TGPI->TG_int(a_bits);
	TGPI->xor_(t18_x, t14_x, t15_x, a_bits, b_bits);
	TGPI->xor_(t18__x, t14_x, t15, a_bits);
	uint64_t t18_chk = TGPI->reveal(t18_x, m_bits, false);
	uint64_t t18__chk = TGPI->reveal(t18__x, a_bits, false);
	if (TGPI->party == BOB) 
		verify_n_report("logical xor of public or secret values", (vector<uint64_t>){t18_chk, t18__chk}, (vector<uint64_t>){t14 ^ t15, t14 ^ t15});

	auto t19_x = TGPI->TG_int(a_bits);
	TGPI->not_(t19_x, t14_x, a_bits);
	int64_t t19_chk = TGPI->reveal(t19_x, a_bits, false);
	uint64_t not_t14 = ~t14;
	for (uint64_t i = a_bits; i < 64; i++)
		not_t14 &= ~(1UL << i); // set the bits beyond a_bits to 0
	if (TGPI->party == BOB) 
		verify_n_report("logical not", t19_chk, not_t14);

	int64_t t20 = -urand_U(), t21 = urand_U();
	cout << "testing relu with a = " << t20 << ", b = " << t21 << endl; 	
	int64_t t20_ref = t20 > 0 ? t20 : 0, t21_ref = t21 > 0 ? t21 : 0;	
	auto t20_x = TGPI->TG_int_init(PUBLIC, a_bits, t20);	
	auto t21_x = TGPI->TG_int_init(PUBLIC, a_bits, t21);	
	TGPI->relu(t20_x, a_bits);	
	TGPI->relu(t21_x, a_bits);	
	int64_t t20_chk = TGPI->reveal(t20_x, a_bits);	
	int64_t t21_chk = TGPI->reveal(t21_x, a_bits);	
	if (TGPI->party == BOB) 	
		verify_n_report("relu", (vector<int64_t>){t20_chk, t21_chk}, (vector<int64_t>){t20_ref, t21_ref});

	TGPI->clear_TG_int (a_x);
	TGPI->clear_TG_int (b_x);
	TGPI->clear_TG_int (t0_x);
	TGPI->clear_TG_int (t1_x);
	TGPI->clear_TG_int (t2_x);
	TGPI->clear_TG_int (t2__x);
	TGPI->clear_TG_int (t3_x);
	TGPI->clear_TG_int (t4_x);
	TGPI->clear_TG_int (t4__x);
	TGPI->clear_TG_int (t5_x);
	TGPI->clear_TG_int (t5__x);
	TGPI->clear_TG_int (t6_x);
	TGPI->clear_TG_int (t6__x);
	TGPI->clear_TG_int (t7_x);
	TGPI->clear_TG_int (t7__x);
	TGPI->clear_TG_int (t8_x);
	TGPI->clear_TG_int (t9_x);
	TGPI->clear_TG_int (t9__x);
	TGPI->clear_TG_int (t10_x);
	TGPI->clear_TG_int (t11_x);
	TGPI->clear_TG_int (t11__x);
	TGPI->clear_TG_int (t12_x);
	TGPI->clear_TG_int (t13_x);
	TGPI->clear_TG_int (t16_x);
	TGPI->clear_TG_int (t16__x);
	TGPI->clear_TG_int (t17_x);
	TGPI->clear_TG_int (t17__x);
	TGPI->clear_TG_int (t18_x);
	TGPI->clear_TG_int (t18__x);
	TGPI->clear_TG_int (t19_x);
	TGPI->clear_TG_int (t20_x);
	TGPI->clear_TG_int (t21_x);
	
	cout << "----- test phase 1: vectors ----" << endl;	
	
	TGPI->retrieve_input_vector_labels(A0_x, ALICE, vector_bits, 8);
	TGPI->retrieve_input_vector_labels(A1_x, ALICE, vector_bits, 4, 3);
	TGPI->retrieve_input_vector_labels(A2_x, ALICE, vector_bits, 2, 3, 4, 5);
	TGPI->retrieve_input_vector_labels(B0_x, BOB, vector_bits, 5, 5, 1, 16);
	TGPI->retrieve_input_vector_labels(B1_x, BOB, vector_bits, 3, 5);	
	TGPI->retrieve_input_vector_labels(B2_x, BOB, vector_bits, 4);		

	TGPI->clear_input_labels(); //all the inputs has been retrieved
	
	auto A0_check = make_vector<int64_t>(8);
	TGPI->reveal_vector(A0_check, A0_x, vector_bits, 8);	
	if (TGPI->party == BOB) 
		verify_n_report("1d vector init by Alice", A0_check, A0);
		
	auto A1_check = make_vector<int64_t>(4, 3);	
	TGPI->reveal_vector(A1_check, A1_x, vector_bits, 4, 3);
	if (TGPI->party == BOB) 
		verify_n_report("2d vector init by Alice", A1_check, A1);	
	
	auto A2_check = make_vector<int64_t>(2, 3, 4, 5);	
	TGPI->reveal_vector(A2_check, A2_x, vector_bits, 2, 3, 4, 5);
	if (TGPI->party == BOB) 
		verify_n_report("3d vector init by Alice", A2_check, A2);			
	
	auto B0_check = make_vector<int64_t>(5, 5, 1, 16);	
	TGPI->reveal_vector(B0_check, B0_x, vector_bits, 5, 5, 1, 16);	
	if (TGPI->party == BOB) 
		verify_n_report("3d vector init by Bob", B0_check, B0);	
		
	auto B1_check = make_vector<int64_t>(3, 5);	
	TGPI->reveal_vector(B1_check, B1_x, vector_bits, 3, 5);
	if (TGPI->party == BOB) 
		verify_n_report("2d vector init by Bob", B1_check, B1);
	
	auto B2_check = make_vector<int64_t>(4);	
	TGPI->reveal_vector(B2_check, B2_x, vector_bits, 4);
	if (TGPI->party == BOB) 
		verify_n_report("1d vector init by Bob", B2_check, B2);

	auto T0_x = TGPI->TG_int_init(PUBLIC, vector_bits, A0, 8);
	auto T1_x = TGPI->TG_int_init(PUBLIC, vector_bits, A1, 4, 3);
	auto T2_x = TGPI->TG_int_init(PUBLIC, vector_bits, A2, 2, 3, 4, 5);
	auto T3_x = TGPI->TG_int(vector_bits, 5, 5, 1, 16);
	auto T4_x = TGPI->TG_int(vector_bits, 3, 5);
	auto T5_x = TGPI->TG_int(vector_bits, 4);
	
	auto T0_chk = make_vector<int64_t>(8);
	TGPI->reveal_vector(T0_chk, T0_x, vector_bits, 8);
	if (TGPI->party == BOB) 
		verify_n_report("1d vector assignment to public", T0_chk, A0);
	
	auto T1_chk = make_vector<int64_t>(4, 3);
	TGPI->reveal_vector(T1_chk, T1_x, vector_bits, 4, 3);
	if (TGPI->party == BOB) 
		verify_n_report("2d vector assignment to public", T1_chk, A1);
	
	auto T2_chk = make_vector<int64_t>(2, 3, 4, 5);
	TGPI->reveal_vector(T2_chk, T2_x, vector_bits, 2, 3, 4, 5);
	if (TGPI->party == BOB) 
		verify_n_report("4d vector assignment to public", T2_chk, A2);

	TGPI->assign_vector(T3_x, B0_x, vector_bits, 5, 5, 1, 16);
	auto T3_chk = make_vector<int64_t>(5, 5, 1, 16);
	TGPI->reveal_vector(T3_chk, T3_x, vector_bits, 5, 5, 1, 16);
	if (TGPI->party == BOB) 
		verify_n_report("4d vector assignment to secret", T3_chk, B0);

	TGPI->assign_vector(T4_x, B1_x, vector_bits, 3, 5);
	auto T4_chk = make_vector<int64_t>(3, 5);
	TGPI->reveal_vector(T4_chk, T4_x, vector_bits, 3, 5);
	if (TGPI->party == BOB) 
		verify_n_report("2d vector assignment to secret", T4_chk, B1);

	TGPI->assign_vector(T5_x, B2_x, vector_bits, 4);
	auto T5_chk = make_vector<int64_t>(4);
	TGPI->reveal_vector(T5_chk, T5_x, vector_bits, 4);
	if (TGPI->party == BOB) 
		verify_n_report("1d vector assignment to secret", T5_chk, B2);

	int64_t t50 = 0;
	auto t50_x = TGPI->TG_int_init(PUBLIC, vector_bits, (int64_t)0);
	for (uint64_t i = 0; i < 8; i++){
		t50 += A0[i];
		TGPI->add(t50_x, t50_x, A0_x[i], vector_bits);
	}
	int64_t t50_chk = TGPI->reveal(t50_x, vector_bits);
	if (TGPI->party == BOB) 
		verify_n_report("1d vector accumulation", t50_chk, t50);
	
	uint64_t bit_width_3 = 15, bit_width_4 = 17, bit_width_5 = 35, bit_width_G = 64, rs_bits = 3;
	uint64_t row_3 = 3, inner = 3, col_4 = 3;
	
	auto T6_x = TGPI->TG_int(bit_width_3, row_3, inner);
	auto T6 = make_vector<int64_t>(row_3, inner);
	input_vector(T6, row_3, inner);
	TGPI->assign_vector(T6_x, T6, bit_width_3, row_3, inner);	
	auto T7_x = TGPI->TG_int(bit_width_4, inner, col_4);
	auto T7 = make_vector<int64_t>(inner, col_4);
	input_vector(T7, inner, col_4);
	TGPI->assign_vector(T7_x, T7, bit_width_4, inner, col_4);
	
	auto T8_x = TGPI->TG_int(bit_width_5, row_3, col_4);	
	TGPI->mat_mult(row_3, inner, col_4, T6_x, T7_x, T8_x, rs_bits, bit_width_3, bit_width_4, bit_width_5, bit_width_G);	
	auto T8_chk = make_vector<int64_t>(row_3, col_4);
	TGPI->reveal_vector(T8_chk, T8_x, bit_width_5, row_3, col_4);	
	auto T8_ref = make_vector<int64_t>(row_3, col_4);
	mat_mult(row_3, inner, col_4, T6, T7, T8_ref, rs_bits);

	if (TGPI->party == BOB) 
		verify_n_report("2d matrix product", T8_chk, T8_ref);
	
	TGPI->clear_TG_int(A0_x, 8);
	TGPI->clear_TG_int(A1_x, 4, 3);
	TGPI->clear_TG_int(A2_x, 2, 3, 4, 5);
	TGPI->clear_TG_int(B0_x, 5, 5, 1, 16);
	TGPI->clear_TG_int(B1_x, 3, 5);
	TGPI->clear_TG_int(B2_x, 4);	
	TGPI->clear_TG_int(T0_x, 8);
	TGPI->clear_TG_int(T1_x, 4, 3);
	TGPI->clear_TG_int(T2_x, 2, 3, 4, 5);
	TGPI->clear_TG_int(T3_x, 5, 5, 1, 16);
	TGPI->clear_TG_int(T4_x, 3, 5);
	TGPI->clear_TG_int(T5_x, 4);
	TGPI->clear_TG_int(T6_x, row_3, inner);
	TGPI->clear_TG_int(T7_x, inner, col_4);
	TGPI->clear_TG_int(T8_x, row_3, col_4);	
	
	delete TGPI;

	return;
}

#endif //PROGRAM_INTERFACE_EXEC_H
