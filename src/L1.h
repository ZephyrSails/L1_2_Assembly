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
  // const int INS_ = 1;
  // const int INS_ = 1;
  // const int INS_ = 1;
  // const int INS_ = 1;
  // const int INS_ = 1;
  // const int INS_ = 1;
  // const int INS_ = 1;
  // const int INS_ = 1;

  const int INS_TYPE_1_ITEM = 1;
  const int INS_TYPE_2_ITEM = 2;
  const int INS_TYPE_CMP    = 3;
  const int INS_TYPE_CJUMP  = 4;
  const int INS_TYPE_CISC   = 5;


  struct L1_item {
    std::string labelName;
  };

  // struct Opperand {
  //   int type;
  //   // 01: assign (movq), {aop}, {sop}, comp, setle, movzbq
  //   // 02:  &
  // };

  struct Item {
    int type;         // 0: register, 1: label, 2: number
    std::string name; // for register & label
    int value;        // for register (offset) & number
  };

  struct Cmp {
    std::vector<L1::Item *> items;
    // L1::Opperand op;
    std::string op;
  };

  struct Instruction {
    int type;     // 0: 0 item, 1: 1 item, 2: 2 items, 3: cmp, 4: cjump, 5: CISC
    std::vector<L1::Item *> items;
    // L1::Opperand op;
    std::string op;
    L1::Cmp cmp;  // 3 & 4 has cmp
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
