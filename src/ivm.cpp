#include <string>
#include "optionparser.h"
#include "IblisASM.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>
//===================== ACTIONS ===========================

/**
 * Assemble the file, placing it in "a.out".
 * @param filename
 */
iblis::Assembler* AssembleFile(std::string filename)
{
	std::ifstream f(filename);
	std::stringstream buffer;
	
	buffer << f.rdbuf();
	f.close();
	
	std::string source = buffer.str();
	return new iblis::Assembler(source);
}

//================== ARG HANDLING =========================
static void printArgError(const char* msg1, const option::Option& opt, const char* msg2)
{
    fprintf(stderr, "%s", msg1);
    fwrite(opt.name, opt.namelen, 1, stderr);
    fprintf(stderr, "%s", msg2);
}

static option::ArgStatus ArgRequired(const option::Option& option, bool msg)
{
    if (option.arg != 0)
		return option::ARG_OK;
	
    if (msg) printArgError("Option '", option, "' requires an argument\n");
    return option::ARG_ILLEGAL;
}

enum OptionIndex {HELP, ASM_FILE};
const option::Descriptor usage[] = {
	{HELP, 0, "h", "help", option::Arg::None, "-h\t --help\t Get usage help."},
	{ASM_FILE, 0, "a", "asm_file", ArgRequired, "-a\t --asm_file\t File to assemble."},
	{0, 0, 0, 0, 0, 0}
};


//==================== MAIN =====================================
int main(int argc, char **argv)
{
	//======set up and read arguments=======
	argc-=(argc>0); argv+=(argc>0);
	option::Stats  stats(usage, argc, argv);
	option::Option* options = new option::Option[stats.options_max];
	option::Option* buffer  = new option::Option[stats.buffer_max];
	option::Parser parse(usage, argc, argv, options, buffer);
	
	if (parse.error()){
		return 1;
	}
	
	if (options[HELP] || argc == 0){
		option::printUsage(std::cout, usage);
	}
	
	
	//===== handle requested operations =====
	if (options[ASM_FILE]){
		iblis::Assembler *as = AssembleFile(options[ASM_FILE].arg);
	}
	
	
	
	
	return 0;
}