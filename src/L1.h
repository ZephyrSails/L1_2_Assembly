#pragma once

#include <vector>

namespace L1 {

  const int ITEM_REGISTER = 0;
  const int ITEM_LABEL = 1;
  const int ITEM_NUMBER = 2;

  const int INS_RETURN = 0;
  const int INS_LABEL = 1;
  const int INS_MEM_OR_W_START = 2;
  const int INS_CALL = 3;
  const int INS_GOTO = 4;
  const int INS_INC_DEC = 5;
  const int INS_CISC = 6;
  const int INS_CMP = 7;
  const int INS_CJUMP = 8;

  struct L1_item {
    std::string labelName;
  };

  struct Item {
    int type;         // defined by L1::ITEM_*TYPE*
    std::string name; // for register & label
    int value;        // for register (offset) & number
  };

  // struct Cmp {
  //   std::vector<L1::Item *> items;
  //   // L1::Opperand op;
  //   std::string op;
  // };

  struct Instruction {
    int type;     // defined by L1::INS_*TYPE*
    std::vector<L1::Item *> items;
    // L1::Opperand op;
    std::string op;
    // L1::Cmp cmp;  // 3 & 4 has cmp
  };

  struct Function {
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<L1::Instruction *> instructions;
  };

  struct Program {
    std::string entryPointLabel;
    std::vector<L1::Function *> functions;
  };

} // L1
