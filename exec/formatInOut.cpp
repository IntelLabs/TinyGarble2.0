#include <emp-tool/emp-tool.h>
#include <boost/program_options.hpp>
#include <boost/lexical_cast/try_lexical_convert.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <cstdlib>

using namespace std;
namespace po = boost::program_options;

std::vector<po::option> ignore_numbers(std::vector<std::string>& args)
{
	std::vector<po::option> result;
	int pos = 0;
	while(!args.empty()) {
		const auto& arg = args[0];
		int64_t num;
		if(boost::conversion::try_lexical_convert(arg, num)) {
			result.push_back(po::option());
			po::option& opt = result.back();

			opt.position_key = pos++;
			opt.value.push_back(arg);
			opt.original_tokens.push_back(arg);

			args.erase(args.begin());
		} else {
			break;
		}
	}

	return result;
}

int main(int argc, char** argv) {
	uint64_t bit_len = 1;
	vector<uint64_t> bit_len_vector;
	vector<int64_t> input;	
	string input_str;
	string output_str;
	vector<int64_t> output;	
	bool is_signed = false;
	
	po::variables_map vm;
	
	po::options_description desc{"Format the input string or parse the output string. \nAllowed options"};
	desc.add_options()  //
	("help,h", "produce help message")  //
	("input,i", po::value<vector<int64_t>>(&input)->multitoken(), "input decimal numbers") //
	("output,o", po::value<string>(&output_str), "output hex string") //
	("bits,b", po::value<vector<uint64_t>>(&bit_len_vector)->multitoken(), "number of bits in each decimal number") //
	("signed,s",  "outputs are parsed as signed numbers (default is unsigned)");
	
	po::store(po::command_line_parser(argc, argv)
	.extra_style_parser(&ignore_numbers)
	.options(desc)
	.run(), vm);
	
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
	
	if (!vm.count("bits"))  error("Error: specify the bit-length of each number.\n");	
	if (bit_len_vector.size() == 1) bit_len = bit_len_vector[0];
	
	if (vm.count("signed")) is_signed = true;
	
	if (vm.count("input")){
		if (bit_len_vector.size() == 1) input_str = formatGCInputString(input, bit_len);
		else{
			if (input.size() != bit_len_vector.size()) error("Error: number of inputs should match the number of bit-lengths.\n");
			input_str = formatGCInputString(input, bit_len_vector);
		}
		cout << input_str << endl;
	}
	
	if (vm.count("output")){
		if (bit_len_vector.size() == 1) parseGCOutputString(output, output_str, bit_len, 0, is_signed);
		else parseGCOutputString(output, output_str, bit_len_vector, 0, is_signed);
		for (uint64_t i = 0; i < output.size(); ++i)
			cout << output[i] << " ";
		cout << endl;
	}
	
	return 0;
}