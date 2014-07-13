#include "AsmGrammar_def.h"
#include "AsmGrammar.h"


typedef iblis::AsmLineIterator iterator_type;
template struct iblis::AsmSkipper<iterator_type>;

typedef iblis::AsmSkipper<iterator_type> skip_type;
template struct iblis::AsmGrammar<iterator_type, skip_type>;