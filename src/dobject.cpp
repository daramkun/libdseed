#include <dseed.h>

dseed::wrapped::~wrapped () { }

dseed::object::object () { memcpy (_signature, "DSEEDOBJ", 8); }
dseed::object::~object () { }
