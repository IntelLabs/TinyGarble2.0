#ifndef __HELPER
#define __HELPER
#include <emp-tool/emp-tool.h>
#include "c2pc_config.h"
#include <immintrin.h>
#include <boost/align/align.hpp>

using std::future;
using std::cout;
using std::endl;
using std::flush;
#undef align

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

template<typename T>
vector<T> make_vector(size_t size) {
	return std::vector<T>(size);
}

template <typename T, typename... Args>
auto make_vector(size_t first, Args... sizes){
	auto inner = make_vector<T>(sizes...);
	return vector<decltype(inner)>(first, inner);
}

template<typename T>
std::ostream& operator<< (std::ostream &os, const vector<T> &v){
	for(auto it = v.begin (); it != v.end (); ++it) {
		os << *it << ", ";
	}
	return os;
}

void input_vector(auto& A, uint64_t len0) {	
	for(uint64_t i0 = 0; i0 < len0; i0++)
		std::cin >> A[i0];				
}
void input_vector(auto& A, uint64_t len0, uint64_t len1) {
	for(uint64_t i0 = 0; i0 < len0; i0++)
		for(uint64_t i1 = 0; i1 < len1; i1++)
			std::cin >> A[i0][i1];
}
void input_vector(auto& A, uint64_t len0, uint64_t len1, uint64_t len2) {
	for(uint64_t i0 = 0; i0 < len0; i0++)
		for(uint64_t i1 = 0; i1 < len1; i1++)
			for (uint64_t i2 = 0; i2 < len2; i2++)
				std::cin >> A[i0][i1][i2];
}		
void input_vector(auto& A, uint64_t len0, uint64_t len1, uint64_t len2, uint64_t len3){
	for(uint64_t i0 = 0; i0 < len0; i0++)
		for(uint64_t i1 = 0; i1 < len1; i1++)
			for (uint64_t i2 = 0; i2 < len2; i2++)
				for (uint64_t i3 = 0; i3 < len3; i3++)
					std::cin >> A[i0][i1][i2][i3];
}	

void dec_vector_to_bin(bool*& bin, vector<int64_t> dec, vector<uint64_t> bit_len){
	uint64_t BIT_LEN = accumulate(bit_len.begin(), bit_len.end(), 0);
	uint64_t size = dec.size();
	uint64_t k = 0;
	for (uint64_t i = 0; i < size; i++){
		std::bitset<64> bits(dec[i]);
		for (uint64_t j = 0; j < bit_len[i]; j++){
			bin[BIT_LEN-1-k] = bits[bit_len[i]-1-j];
			k++;
		}
	}
}

namespace emp {

void send_bool_aligned(NetIO* io, const bool * data, int length) {
	unsigned long long * data64 = (unsigned long long * )data;
	int i = 0;
#if !defined(__BMI2__)
	unsigned long long mask;
#endif
	for(; i < length/8; ++i) {
		unsigned long long tmp;
#if defined(__BMI2__)
		tmp = _pext_u64(data64[i], 0x0101010101010101ULL);
#else
		// https://github.com/Forceflow/libmorton/issues/6
		tmp = 0;
		mask = 0x0101010101010101ULL;
		for (unsigned long long bb = 1; mask != 0; bb += bb) {
			if (data64[i] & mask & -mask) { tmp |= bb; }
			mask &= (mask - 1);
		}
#endif
		io->send_data(&tmp, 1);
	}
	if (8*i != length)
		io->send_data(data + 8*i, length - 8*i);
}
void recv_bool_aligned(NetIO* io, bool * data, int length) {
	unsigned long long * data64 = (unsigned long long *) data;
	int i = 0;
#if !defined(__BMI2__)
	unsigned long long mask;
#endif
	for(; i < length/8; ++i) {
		unsigned long long tmp = 0;
		io->recv_data(&tmp, 1);
#if defined(__BMI2__)
		data64[i] = _pdep_u64(tmp, (unsigned long long) 0x0101010101010101ULL);
#else
		data64[i] = 0;
		mask = 0x0101010101010101ULL;
                for (unsigned long long bb = 1; mask != 0; bb += bb) {
                        if (tmp & bb) {data64[i] |= mask & (-mask); }
                        mask &= (mask - 1);
                }
#endif
	}
	if (8*i != length)
		io->recv_data(data + 8*i, length - 8*i);
}
void send_bool(NetIO * io, bool * data, int length) {
	void * ptr = (void *)data;
	size_t space = length;
	void * aligned = boost::alignment::align(alignof(uint64_t), sizeof(uint64_t), ptr, space);
    if(aligned == nullptr)
        io->send_data(data, length);
    else{
        int diff = length - space;
        io->send_data(data, diff);
        send_bool_aligned(io, (const bool*)aligned, length - diff);
    }
}

void recv_bool(NetIO * io, bool * data, int length) {
	void * ptr = (void *)data;
	size_t space = length;
	void * aligned = boost::alignment::align(alignof(uint64_t), sizeof(uint64_t), ptr, space);
    if(aligned == nullptr)
        io->recv_data(data, length);
    else{
        int diff = length - space;
        io->recv_data(data, diff);
        recv_bool_aligned(io, (bool*)aligned, length - diff);
    }
}

template<int B>
void send_partial_block(NetIO * io, const block * data, int length) {
	for(int i = 0; i < length; ++i) {
		io->send_data(&(data[i]), B);
	}
}

template<int B>
void recv_partial_block(NetIO * io, block * data, int length) {
	for(int i = 0; i < length; ++i) {
		io->recv_data(&(data[i]), B);
	}
}
}
#endif// __HELPER
