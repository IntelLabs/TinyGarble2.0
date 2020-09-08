#include <emp-tool/emp-tool.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include "tinygarble/program_interface_sh.h"
#include "tinygarble/cnn_layers.h"

using namespace std;
namespace po = boost::program_options;

void LeNet(NetIO* io, int party) {

	cout << "Privacy-Preserving inference with the CNN LeNet" << endl;
	
	TinyGarblePI_SH* TGPI_SH = new TinyGarblePI_SH(io, party);
	io->flush();

	uint64_t bit_width_global = 64;

	TinyGarbleCNN* TGCNN = new TinyGarbleCNN(TGPI_SH, bit_width_global);
	
	vector<uint64_t> bit_width(50, 0);	
	bit_width[0]  = 17;
	bit_width[1]  = 15;
	bit_width[2]  = 14;
	bit_width[3]  = 14;
	bit_width[4]  = 13;
	bit_width[5]  = 15;
	bit_width[6]  = 13;
	bit_width[7]  = 14;
	bit_width[8]  = 13;
	bit_width[10] = 17;
	bit_width[12] = 20;
	bit_width[15] = 20;
	bit_width[18] = 20;
	bit_width[20] = 20;
	bit_width[22] = 24;
	bit_width[25] = 24;
	bit_width[28] = 24;
	bit_width[30] = 24;
	bit_width[33] = 24;
	bit_width[35] = 28;
	bit_width[38] = 28;
	bit_width[41] = 27;
	bit_width[43] = 30;
	bit_width[46] = 30;
	bit_width[49] = 5;	
	
	memMeter M;
	Timer T;	
	T.start();

	auto t0 = make_vector<int64_t> (1, 784);
	auto t1 = make_vector<int64_t> (5, 5, 1, 16);
	auto t2 = make_vector<int64_t> (16);
	auto t3 = make_vector<int64_t> (5, 5, 16, 16);
	auto t4 = make_vector<int64_t> (16);
	auto t5 = make_vector<int64_t> (256, 100);
	auto t6 = make_vector<int64_t> (100);
	auto t7 = make_vector<int64_t> (100, 10);
	auto t8 = make_vector<int64_t> (10);
	
	if (TGPI_SH->party == BOB){
		input_vector(t0, 1, 784);
	}
	else{
		input_vector(t1, 5, 5, 1, 16);
		input_vector(t2, 16);
		input_vector(t3, 5, 5, 16, 16);
		input_vector(t4, 16);
		input_vector(t5, 256, 100);
		input_vector(t6, 100);
		input_vector(t7, 100, 10);
		input_vector(t8, 10);
	}
	
	auto t0_x = TGPI_SH->TG_int_init(BOB, bit_width[0], t0, 1, 784);
	auto t1_x = TGPI_SH->TG_int_init(ALICE, bit_width[1], t1, 5, 5, 1, 16);
	auto t2_x = TGPI_SH->TG_int_init(ALICE, bit_width[2], t2, 16);
	auto t3_x = TGPI_SH->TG_int_init(ALICE, bit_width[3], t3, 5, 5, 16, 16);
	auto t4_x = TGPI_SH->TG_int_init(ALICE, bit_width[4], t4, 16);
	auto t5_x = TGPI_SH->TG_int_init(ALICE, bit_width[5], t5, 256, 100);
	auto t6_x = TGPI_SH->TG_int_init(ALICE, bit_width[6], t6, 100);
	auto t7_x = TGPI_SH->TG_int_init(ALICE, bit_width[7], t7, 100, 10);
	auto t8_x = TGPI_SH->TG_int_init(ALICE, bit_width[8], t8, 10);
	
	M.print("register_input_vector");
	
	TGPI_SH->gen_input_labels();
	
	M.print("gen_input_labels");
	
	TGPI_SH->retrieve_input_vector_labels(t0_x, BOB, bit_width[0], 1, 784);
	TGPI_SH->retrieve_input_vector_labels(t1_x, ALICE, bit_width[1], 5, 5, 1, 16);
	TGPI_SH->retrieve_input_vector_labels(t2_x, ALICE, bit_width[2], 16);
	TGPI_SH->retrieve_input_vector_labels(t3_x, ALICE, bit_width[3], 5, 5, 16, 16);
	TGPI_SH->retrieve_input_vector_labels(t4_x, ALICE, bit_width[4], 16);
	TGPI_SH->retrieve_input_vector_labels(t5_x, ALICE, bit_width[5], 256, 100);
	TGPI_SH->retrieve_input_vector_labels(t6_x, ALICE, bit_width[6], 100);
	TGPI_SH->retrieve_input_vector_labels(t7_x, ALICE, bit_width[7], 100, 10);
	TGPI_SH->retrieve_input_vector_labels(t8_x, ALICE, bit_width[8], 10);

	M.print("retrieve_input_vector_labels");	
	
	TGPI_SH->clear_input_labels();
	
	T.print("Input labels");
	M.print("");


	auto t10_x = TGPI_SH->TG_int(bit_width[10], 1, 28, 28, 1);
	TGCNN->Reshape2Dto4D_TG(1, 784, 1, 28, 28, 1, t0_x, t10_x, bit_width[10]);
	TGPI_SH->clear_TG_int(t0_x, 1, 784);
	
	T.print("Reshape input image");
	M.print("");
	
	auto t12_x = TGPI_SH->TG_int(bit_width[12], 1, 24, 24, 16);
	TGCNN->Conv2DCSF_TG(1, 28, 28, 1, 5, 5, 16, 0, 0, 0, 0, 1, 1, t10_x, t1_x, 12, t12_x, bit_width[10], bit_width[1], bit_width[12]);
	//t10_x, t1_x cleared inside Conv2DCSF_TG
	
	T.print("1st convolution layer");
	M.print("");

	auto t15_x = TGPI_SH->TG_int(bit_width[15], 1, 24, 24, 16);
	TGCNN->MatAddBroadCast4_TG(1, 24, 24, 16, t12_x, t2_x, t15_x, bit_width[12], bit_width[2], bit_width[15]);
	TGPI_SH->clear_TG_int(t2_x, 16);
	TGPI_SH->clear_TG_int(t12_x, 1, 24, 24, 16);
	
	T.print("Add bias");
	M.print("");

	auto t18_x = TGPI_SH->TG_int(bit_width[18], 1, 12, 12, 16);
	TGCNN->MaxPool_TG(1, 12, 12, 16, 2, 2, 0, 0, 0, 0, 2, 2, 1, 24, 24, 16, t15_x, t18_x, bit_width[18]);
	TGPI_SH->clear_TG_int(t15_x, 1, 24, 24, 16);
	
	T.print("Maxpool");
	M.print("");
	
	TGCNN->Relu4_TG(1, 12, 12, 16, t18_x, bit_width[18]);
	
	T.print("ReLU");
	M.print("");

	auto t22_x = TGPI_SH->TG_int(bit_width[22], 1, 8, 8, 16);
	TGCNN->Conv2DCSF_TG(1, 12, 12, 16, 5, 5, 16, 0, 0, 0, 0, 1, 1, t18_x, t3_x, 12, t22_x, bit_width[18], bit_width[3], bit_width[22]);
	
	T.print("2nd convolution layer");
	M.print("");

	auto t25_x = TGPI_SH->TG_int(bit_width[25], 1, 8, 8, 16);
	TGCNN->MatAddBroadCast4_TG(1, 8, 8, 16, t22_x, t4_x, t25_x, bit_width[22], bit_width[4], bit_width[25]);
	TGPI_SH->clear_TG_int(t22_x, 1, 8, 8, 16);
	TGPI_SH->clear_TG_int(t4_x, 16);
	
	T.print("Add bias");
	M.print("");
		
	auto t28_x = TGPI_SH->TG_int(bit_width[25], 1, 4, 4, 16);
	TGCNN->MaxPool_TG(1, 4, 4, 16, 2, 2, 0, 0, 0, 0, 2, 2, 1, 8, 8, 16, t25_x, t28_x, bit_width[25]);
	TGPI_SH->clear_TG_int(t25_x, 1, 8, 8, 16);
	
	T.print("Maxpool");
	M.print("");
	
	TGCNN->Relu4_TG(1, 4, 4, 16, t28_x, bit_width[28]);	
	
	T.print("ReLU");
	M.print("");

	auto t33_x = TGPI_SH->TG_int(bit_width[33], 1, 256);
	TGCNN->Reshape4Dto2D_TG(1, 4, 4, 16, 1, 256, t28_x, t33_x, bit_width[33]);
	TGPI_SH->clear_TG_int(t28_x, 1, 4, 4, 16);	
	
	T.print("Reshape tensor");
	M.print("");
	
	auto t35_x = TGPI_SH->TG_int(bit_width[35], 1, 100);
	TGCNN->MatMulCSF2D_TG(1, 256, 100, t33_x, t5_x, t35_x, 12, bit_width[33], bit_width[5], bit_width[35]);
	TGPI_SH->clear_TG_int(t33_x, 1, 256);
	TGPI_SH->clear_TG_int(t5_x, 256, 100);
	
	T.print("1st FC layer");
	M.print("");

	auto t38_x = TGPI_SH->TG_int(bit_width[38], 1, 100);
	TGCNN->MatAddBroadCast2_TG(1, 100, t35_x, t6_x, t38_x, bit_width[35], bit_width[6], bit_width[38]);
	TGPI_SH->clear_TG_int(t35_x, 1, 100);
	TGPI_SH->clear_TG_int(t6_x, 100);
	
	T.print("Add bias");
	M.print("");

	TGCNN->Relu2_TG(1, 100, t38_x, bit_width[38]);
	
	T.print("ReLU");
	M.print("");
	
	auto t43_x = TGPI_SH->TG_int(bit_width[43], 1, 10);
	TGCNN->MatMulCSF2D_TG(1, 100, 10, t38_x, t7_x, t43_x, 12, bit_width[38], bit_width[7], bit_width[43]);
	TGPI_SH->clear_TG_int(t38_x, 1, 100);
	TGPI_SH->clear_TG_int(t7_x, 100, 10);
	
	T.print("2nd FC layer");
	M.print("");

	auto t46_x = TGPI_SH->TG_int(bit_width[46], 1, 10);
	TGCNN->MatAddBroadCast2_TG(1, 10, t43_x, t8_x, t46_x, bit_width[43], bit_width[8], bit_width[46]);
	TGPI_SH->clear_TG_int(t43_x, 1, 10);
	TGPI_SH->clear_TG_int(t8_x, 10);
	
	T.print("Add bias");
	M.print("");
	
	auto t49_x = TGPI_SH->TG_int(bit_width[49]);
	TGCNN->ArgMax1_TG(1, 1, 10, t46_x, 1, t49_x, bit_width[46], bit_width[49]);
	TGPI_SH->clear_TG_int(t46_x, 1, 10);
	int64_t t49 = TGPI_SH->reveal(t49_x, bit_width[49], 1);
	
	T.print("Argmax1");
	M.print("");
	
	cout << ("Inference Result:") << endl;
	cout << (t49) << endl;
	
	T.print("Total");

	M.print("End");
	cout << "Transferred Data: " << (float)(TGPI_SH->io->num_bytes_sent) / 1024 / 1024 / 1024 << "GB" << endl;
	
	delete TGPI_SH;
	delete TGCNN;
	
	return;
}



int main(int argc, char** argv) {
	int party = 1, port = 1234;
	string netlist_address;
	string server_ip;
	int program;
	
	po::options_description desc{"Privacy-Preserving inference with the CNN LeNet \nAllowed options"};
	desc.add_options()  //
	("help,h", "produce help message")  //
	("party,k", po::value<int>(&party)->default_value(1), "party id: 1 for garbler, 2 for evaluator")  //
	("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
	("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"), "server's IP.");
	
	po::variables_map vm;
	try {
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
		po::store(parsed, vm);
		if (vm.count("help")) {
			cout << desc << endl;
			return 0;
		}
		po::notify(vm);
	}catch (po::error& e) {
		cout << "ERROR: " << e.what() << endl << endl;
		cout << desc << endl;
		return -1;
	}
		
	NetIO* io = new NetIO(party==ALICE ? nullptr:server_ip.c_str(), port, true);
	io->set_nodelay();

    LeNet(io, party);
	
	delete io;
	
	return 0;
}