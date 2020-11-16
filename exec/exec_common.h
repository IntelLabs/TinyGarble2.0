#ifndef EXEC_COMMON_H
#define EXEC_COMMON_H

#include "tinygarble/program_interface.h"
#include "tinygarble/program_interface_sh.h"

#define U 30000
#define L (-U)
inline int64_t rand_L_U(){return ((int64_t)(L + rand()%(U-L+1)));}
inline int64_t urand_U(){return ((int64_t)(rand()%(U+1)));}

void mat_mult(uint64_t i, uint64_t j, uint64_t k, auto &A, auto &B, auto &C, int64_t rs_bits){
	for (uint64_t i1 = 0; i1 < i; i1++){
		for (uint64_t i2 = 0; i2 < k; i2++){
			C[i1][i2] = (int64_t) 0;
			for (uint64_t i3 = 0; i3 < j; i3++){
				C[i1][i2] = (C[i1][i2] + (A[i1][i3] *B[i3][i2]));
			}
			C[i1][i2] = (C[i1][i2] >> rs_bits);
		}
	}
}

uint64_t test_index = 0, test_passed = 0;
void verify_n_report(string test, auto res, auto ref){
    cout << "[<#>] test " << test_index++ << ": " << test;
    if (res == ref){
		test_passed++;
		cout << ": success!";
	}
    else cout << ": fail! excpected: " << ref << " computed: " << res;
	cout << "(" << test_passed << "/" << test_index << ")" << endl;
}

#endif //EXEC_COMMON_H