#include "AsmGrammar_def.h"
#include "AsmGrammar.h"


typedef std::string::const_iterator iterator_type;
template struct iblis::AsmSkipper<iterator_type>;

typedef iblis::AsmSkipper<iterator_type> skip_type;
template struct iblis::AsmGrammar<iterator_type, skip_type>;