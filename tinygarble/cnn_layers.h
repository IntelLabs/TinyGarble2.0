#ifndef CNNLAYERS_TG_H 
#define CNNLAYERS_TG_H

#include <emp-tool/emp-tool.h>
#include "tinygarble/program_interface_sh.h"

class TinyGarbleCNN{
	public:
	TinyGarblePI_SH* TGPI_SH;
	uint64_t bit_width_global;
	
	TinyGarbleCNN(TinyGarblePI_SH* TGPI_SH, uint64_t bit_width_global) {
		this->TGPI_SH = TGPI_SH;
		this->bit_width_global = bit_width_global;	
	}		

	void Reshape2Dto4D_TG(uint64_t inlen0, uint64_t inlen1, uint64_t outlen0, uint64_t outlen1, uint64_t outlen2, uint64_t outlen3, auto inArr, auto& ourArr, uint64_t bit_width){
		if(inlen0*inlen1 != outlen0*outlen1*outlen2*outlen3){
			printf("vector shape mismatch\n");
			exit(1);
		}
		int i5, i4, i3, i2, i1 = 0, i0 = 0;
		for (i2 = 0; i2 < outlen0; i2++){
			for (i3 = 0; i3 < outlen1; i3++){
				for (i4 = 0; i4 < outlen2; i4++){
					for (i5 = 0; i5 < outlen3; i5++){
						TGPI_SH->assign(ourArr[i2][i3][i4][i5], inArr[i0][i1], bit_width);
						i1 = (i1 +  1);
						int __tac_var282 = (i1 ==  inlen1);
						if (__tac_var282) {
							i1 =  0;
							i0 = (i0 +  1);
						}
					}
				}
			}
		}
	}

	void Reshape4Dto2D_TG(int inlen0, int inlen1, int inlen2, int inlen3, int outlen0, int outlen1, auto inArr, auto& ourArr, uint64_t bit_width){
		if(inlen0*inlen1*inlen2*inlen3 != outlen0*outlen1){
			printf("vector shape mismatch\n");
			exit(1);
		}	
		int i11, i10, i9 = 0, i8 = 0, i7 = 0, i6 = 0;
		for (int i10 = 0; i10 < outlen0; i10++){
			for (int i11 = 0; i11 < outlen1; i11++){
				TGPI_SH->assign(ourArr[i10][i11], inArr[i6][i7][i8][i9], bit_width);
				i9 = (i9 + 1);
				int __tac_var283 = (i9 == inlen3);
				if (__tac_var283) {
					i9 = 0;
					i8 = (i8 + 1);
					int __tac_var284 = (i8 == inlen2);
					if (__tac_var284) {
						i8 = 0;
						i7 = (i7 + 1);
						int __tac_var285 = (i7 == inlen1);
						if (__tac_var285) {
							i7 = 0;
							i6 = (i6 + 1);
						}
					}
				}
			}
		}
	}

	void MatMulCSF2D_TG(int32_t i, int32_t j, int32_t k, auto &A, auto &B, auto &C, int64_t consSF, uint64_t bit_width_A, uint64_t bit_width_B, uint64_t bit_width_C){
		cout << "matrix multiplication " << i << ":" << j << ":" << k << endl;
		TGPI_SH->mat_mult(i, j, k, A, B, C, consSF, bit_width_A, bit_width_B, bit_width_C, bit_width_global);
	#if 0	
		uint64_t bit_width_AB = bit_width_A + bit_width_B - 1;
		auto AB = TGPI_SH->TG_int(bit_width_AB);
		auto c = TGPI_SH->TG_int(bit_width_global); //may need larger bit-width to contain intermediate values
		for (uint32_t i1 = 0; i1 < i; i1++){
			for (uint32_t i2 = 0; i2 < k; i2++){
				TGPI_SH->assign(c, (int64_t)0, bit_width_global);
				for (uint32_t i3 = 0; i3 < j; i3++){
					TGPI_SH->mult(AB, A[i1][i3], B[i3][i2], /*bit_width_AB, */bit_width_A, bit_width_B);
					TGPI_SH->add(c, c, AB, bit_width_global, bit_width_AB);
				}
				TGPI_SH->right_shift(c, consSF, bit_width_global);
				TGPI_SH->assign(C[i1][i2], c, bit_width_C);
				cout << fixed << setprecision(2) << setfill('0');
				cout << "\r   " << ((double)(i1*k+i2+1))/(i*k)*100 << "%";
			}
		}
		cout << endl;
	#endif
	}

	void ArgMax1_TG(int32_t outArrS1, int32_t inArrS1, int32_t inArrS2, auto &inArr, int32_t dim, auto &outArr, uint64_t bit_width_in, uint64_t bit_width_out){
		auto maxi_x = TGPI_SH->TG_int(bit_width_in);
		auto maxiIdx_x = TGPI_SH->TG_int(bit_width_out);
		auto i_x = TGPI_SH->TG_int(bit_width_out);
		auto b_x = TGPI_SH->TG_int(1);
		
		for (uint32_t od = 0; od < inArrS1; od++){
			TGPI_SH->assign(maxi_x, inArr[od][0], bit_width_in);
			TGPI_SH->assign(maxiIdx_x, (int64_t)0, bit_width_out);
			for (uint32_t i = 0; i < inArrS2; i++){
				TGPI_SH->assign(i_x, (int64_t)i, bit_width_out);
				TGPI_SH->lt(b_x, maxi_x, inArr[od][i], bit_width_in);
				TGPI_SH->ifelse(maxiIdx_x, b_x, i_x, maxiIdx_x, bit_width_out);
				TGPI_SH->ifelse(maxi_x, b_x, inArr[od][i], maxi_x, bit_width_in);
			}
			TGPI_SH->assign(outArr, maxiIdx_x, bit_width_out);
		}

		TGPI_SH->clear_TG_int(maxi_x);
		TGPI_SH->clear_TG_int(maxiIdx_x);
		TGPI_SH->clear_TG_int(i_x);
		TGPI_SH->clear_TG_int(b_x);
	}

	void Relu2_TG(int32_t s1, int32_t s2, auto &inArr, uint64_t bit_width){
		for (uint32_t i1 = 0; i1 < s1; i1++){
			for (uint32_t i2 = 0; i2 < s2; i2++){
				TGPI_SH->relu(inArr[i1][i2], bit_width);
			}
		}
	}

	void Relu4_TG(int32_t s1, int32_t s2, int32_t s3, int32_t s4, auto &inArr, uint64_t bit_width){
		for (uint32_t i1 = 0; i1 < s1; i1++){
			for (uint32_t i2 = 0; i2 < s2; i2++){
				for (uint32_t i3 = 0; i3 < s3; i3++){
					for (uint32_t i4 = 0; i4 < s4; i4++){
						TGPI_SH->relu(inArr[i1][i2][i3][i4], bit_width);
					}
				}
			}
		}
	}

	void MaxPool_TG(int32_t N, int32_t H, int32_t W, int32_t C, int32_t ksizeH, int32_t ksizeW, int32_t zPadHLeft, int32_t zPadHRight, int32_t zPadWLeft, int32_t zPadWRight, int32_t strideH, int32_t strideW, int32_t N1, int32_t imgH, int32_t imgW, int32_t C1, auto &inArr, auto &outArr, uint64_t bit_width){
		auto maxi_x = TGPI_SH->TG_int(bit_width);
		auto temp_x = TGPI_SH->TG_int(bit_width);
		
		for (uint32_t n = 0; n < N; n++){
			for (uint32_t c = 0; c < C; c++){
				int32_t leftTopCornerH = (0 - zPadHLeft);
				int32_t extremeRightBottomCornerH = ((imgH - 1) + zPadHRight);
				int32_t ctH = 0;
				while ((((leftTopCornerH + ksizeH) - 1) <= extremeRightBottomCornerH)){
					int32_t leftTopCornerW = (0 - zPadWLeft);
					int32_t extremeRightBottomCornerW = ((imgW - 1) + zPadWRight);
					int32_t ctW = 0;
					while ((((leftTopCornerW + ksizeW) - 1) <= extremeRightBottomCornerW)){
						TGPI_SH->assign(maxi_x, (int64_t)0, bit_width);
						if ((((leftTopCornerH < 0) || (leftTopCornerH >= imgH)) || ((leftTopCornerW < 0) || (leftTopCornerW >= imgW)))){
							TGPI_SH->assign(maxi_x, (int64_t)0, bit_width);
						}
						else{
							TGPI_SH->assign(maxi_x, inArr[n][leftTopCornerH][leftTopCornerW][c], bit_width);
						}
						for (uint32_t fh = 0; fh < ksizeH; fh++){
							for (uint32_t fw = 0; fw < ksizeW; fw++){
								int32_t curPosH = (leftTopCornerH + fh);
								int32_t curPosW = (leftTopCornerW + fw);
								TGPI_SH->assign(temp_x, (int64_t)0, bit_width);
								if ((((curPosH < 0) || (curPosH >= imgH)) || ((curPosW < 0) || (curPosW >= imgW)))){
									TGPI_SH->assign(temp_x, (int64_t)0, bit_width);
								}
								else{
									TGPI_SH->assign(temp_x, inArr[n][curPosH][curPosW][c], bit_width);
								}
								TGPI_SH->max(maxi_x, maxi_x, temp_x, bit_width);
							}
						}
						TGPI_SH->assign(outArr[n][ctH][ctW][c], maxi_x, bit_width);
						leftTopCornerW = (leftTopCornerW + strideW);
						ctW = (ctW + 1);
					}
					leftTopCornerH = (leftTopCornerH + strideH);
					ctH = (ctH + 1);
				}
			}
		}

		TGPI_SH->clear_TG_int(maxi_x);
		TGPI_SH->clear_TG_int(temp_x);
	}

	void MatAddBroadCast2_TG(int32_t s1, int32_t s2, auto &A, auto &B, auto &outArr, uint64_t bit_width_A, uint64_t bit_width_B, uint64_t bit_width_out){
		for (uint32_t i1 = 0; i1 < s1; i1++){
			for (uint32_t i2 = 0; i2 < s2; i2++){
				TGPI_SH->add(outArr[i1][i2], A[i1][i2], B[i2], bit_width_A, bit_width_B);
			}
		}
	}

	void MatAddBroadCast4_TG(int32_t s1, int32_t s2, int32_t s3, int32_t s4, auto &A, auto &B, auto &outArr, uint64_t bit_width_A, uint64_t bit_width_B, uint64_t bit_width_out){
		for (uint32_t i1 = 0; i1 < s1; i1++){
			for (uint32_t i2 = 0; i2 < s2; i2++){
				for (uint32_t i3 = 0; i3 < s3; i3++){
					for (uint32_t i4 = 0; i4 < s4; i4++){					
						TGPI_SH->add(outArr[i1][i2][i3][i4], A[i1][i2][i3][i4], B[i4], bit_width_A, bit_width_B);
					}
				}
			}
		}
	}

	void Conv2DReshapeFilter_TG(int32_t FH, int32_t FW, int32_t CI, int32_t CO, auto &inputArr, auto &outputArr, uint64_t bit_width_filter){
		for (uint32_t co = 0; co < CO; co++){
			for (uint32_t fh = 0; fh < FH; fh++){
				for (uint32_t fw = 0; fw < FW; fw++){
					for (uint32_t ci = 0; ci < CI; ci++){
						int32_t linIdx = ((((fh *FW) *CI) + (fw *CI)) + ci);
						TGPI_SH->assign(outputArr[co][linIdx], inputArr[fh][fw][ci][co], bit_width_filter);
					}
				}
			}
		}
	}

	void Conv2DReshapeMatMulOP_TG(int32_t N, int32_t finalH, int32_t finalW, int32_t CO, auto &inputArr, auto &outputArr, uint64_t bit_width_out){
		for (uint32_t co = 0; co < CO; co++){
			for (uint32_t n = 0; n < N; n++){
				for (uint32_t h = 0; h < finalH; h++){
					for (uint32_t w = 0; w < finalW; w++){
						TGPI_SH->assign(outputArr[n][h][w][co], inputArr[co][((((n *finalH) *finalW) + (h *finalW)) + w)], bit_width_out);
					}
				}
			}
		}
	}

	void Conv2DReshapeInput_TG(int32_t N, int32_t H, int32_t W, int32_t CI, int32_t FH, int32_t FW, int32_t zPadHLeft, int32_t zPadHRight, int32_t zPadWLeft, int32_t zPadWRight, int32_t strideH, int32_t strideW, int32_t RRows, int32_t RCols, auto &inputArr, auto &outputArr, uint64_t bit_width_input){
		int32_t linIdxFilterMult = 0;
		for (uint32_t n = 0; n < N; n++){
			int32_t leftTopCornerH = (0 - zPadHLeft);
			int32_t extremeRightBottomCornerH = ((H - 1) + zPadHRight);
			while ((((leftTopCornerH + FH) - 1) <= extremeRightBottomCornerH)){
				int32_t leftTopCornerW = (0 - zPadWLeft);
				int32_t extremeRightBottomCornerW = ((W - 1) + zPadWRight);
				while ((((leftTopCornerW + FW) - 1) <= extremeRightBottomCornerW)){
					for (uint32_t fh = 0; fh < FH; fh++){
						for (uint32_t fw = 0; fw < FW; fw++){
							int32_t curPosH = (leftTopCornerH + fh);
							int32_t curPosW = (leftTopCornerW + fw);
							for (uint32_t ci = 0; ci < CI; ci++){
								if ((((curPosH < 0) || (curPosH >= H)) || ((curPosW < 0) || (curPosW >= W)))){
									TGPI_SH->assign(outputArr[((((fh *FW) *CI) + (fw *CI)) + ci)][linIdxFilterMult], (int64_t)0, bit_width_input);
								}
								else{
									TGPI_SH->assign(outputArr[((((fh *FW) *CI) + (fw *CI)) + ci)][linIdxFilterMult], inputArr[n][curPosH][curPosW][ci], bit_width_input);
								}
							}
						}
					}
					linIdxFilterMult = (linIdxFilterMult + 1);
					leftTopCornerW = (leftTopCornerW + strideW);
				}
				leftTopCornerH = (leftTopCornerH + strideH);
			}
		}
	}

	void Conv2DCSF_TG(int32_t N, int32_t H, int32_t W, int32_t CI, int32_t FH, int32_t FW, int32_t CO, int32_t zPadHLeft, int32_t zPadHRight, int32_t zPadWLeft, int32_t zPadWRight, int32_t strideH, int32_t strideW, auto &inputArr, auto &filterArr, int32_t consSF, auto &outArr, uint64_t bit_width_input, uint64_t bit_width_filter, uint64_t bit_width_out)
	{
		int32_t reshapedFilterRows = CO;
		int32_t reshapedFilterCols = ((FH *FW) *CI);
		int32_t reshapedIPRows = ((FH *FW) *CI);
		int32_t newH = ((((H + (zPadHLeft + zPadHRight)) - FH) / strideH) + 1);
		int32_t newW = ((((W + (zPadWLeft + zPadWRight)) - FW) / strideW) + 1);
		int32_t reshapedIPCols = ((N *newH) *newW);

		auto filterReshaped = TGPI_SH->TG_int(bit_width_filter, reshapedFilterRows, reshapedFilterCols);
		auto inputReshaped = TGPI_SH->TG_int(bit_width_input, reshapedIPRows, reshapedIPCols);
		auto matmulOP = TGPI_SH->TG_int(bit_width_out, reshapedFilterRows, reshapedIPCols);
		
		Conv2DReshapeFilter_TG(FH, FW, CI, CO, filterArr, filterReshaped, bit_width_filter);
		TGPI_SH->clear_TG_int(filterArr, FH, FW, CI, CO); 
		
		Conv2DReshapeInput_TG(N, H, W, CI, FH, FW, zPadHLeft, zPadHRight, zPadWLeft, zPadWRight, strideH, strideW, reshapedIPRows, reshapedIPCols, inputArr, inputReshaped, bit_width_input);
		TGPI_SH->clear_TG_int(inputArr, N, H, W, CI); 
		
		MatMulCSF2D_TG(reshapedFilterRows, reshapedFilterCols, reshapedIPCols, filterReshaped, inputReshaped, matmulOP, consSF, bit_width_filter, bit_width_input, bit_width_out);
		TGPI_SH->clear_TG_int(filterReshaped, reshapedFilterRows, reshapedFilterCols);
		TGPI_SH->clear_TG_int(inputReshaped, reshapedIPRows, reshapedIPCols);
		
		Conv2DReshapeMatMulOP_TG(N, newH, newW, CO, matmulOP, outArr, bit_width_out);
		TGPI_SH->clear_TG_int(matmulOP, reshapedFilterRows, reshapedIPCols); 
	}
};

# endif	//CNNLAYERS_TG_H