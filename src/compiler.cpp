#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <fstream>
#include <map>

#include <parser.h>

using namespace std;

std::map<std::string, std::string> init_op_map() {
  std::map<std::string, std::string> op_map;
  op_map["<-"] = "movq";
  op_map["+="] = "addq";
  op_map["-="] = "subq";
  op_map["*="] = "imulq";
  op_map["&="] = "andq";
  op_map["<<="] = "salq";
  op_map[">>="] = "sarq";
  op_map["++"] = "inc";
  op_map["--"] = "dec";
  return op_map;
}

void return_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
  if (f->locals > 0) {
    *outputFile << "\n\taddq $" << std::to_string(f->locals * 8) << ", %rsp";
  }
  *outputFile << "\n\tretq";
}

void label_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
  *outputFile << "\n\n_" << i->items.at(0)->name << ":";
}

std::string item2string(L1::Item * i, bool isLeft = false) {
  std::string str;
  switch (i->type) {
    case L1::ITEM_LABEL:
      if (isLeft) {
        str = "$_" + i->name;
      } else {
        str = "_" + i->name;
      }

      break;
    case L1::ITEM_REGISTER:
      if (i->value == -1) {
        str = "%" + i->name;
      } else {
        str = std::to_string(i->value) + "(%" + i->name + ")";
      }
      break;
    case L1::ITEM_NUMBER:
      str = "$" + std::to_string(i->value);
  }
  return str;
}

void L1_ins_two_op(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f, std::map<std::string, std::string> op_map) {
  L1::Item * left_item = i->items.at(1);


  *outputFile << "\n\t" << op_map[i->op] << " " << item2string(left_item, true) << ", " << item2string(i->items.at(0));
}

void call_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
  L1::Item * item = i->items.at(0);
  if (item->name.at(0) == ':' || item->name.at(0) == 'r') {  // self defined func
    int rsp_offset;
    if (item->name.at(0) == ':') {
      item->name.erase(0, 1);
      item->name.insert(0, "_");
    } else {
      item->name.insert(0, "*%");
    }

    if (item->value > 6) {
      // cout << "\n--------- value > 6";
      rsp_offset = (item->value - 5) * 8;
    } else {
      rsp_offset = 8;
    }
    *outputFile << "\n\tsubq $" << std::to_string(rsp_offset) << ", %rsp";
    *outputFile << "\n\tjmp " << item->name;
  } else { // system func
    // cout << "\n--------- value <= 6";
    *outputFile << "\n\tcall " << item->name;
  }
}

void goto_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
  L1::Item *item = i->items.at(0);
  item->name.erase(0, 1);
  item->name.insert(0, "_");
  *outputFile << "\n\tjmp " << item->name;
}

void inc_dec_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f, std::map<std::string, std::string> op_map) {
  *outputFile << "\n\t" << op_map[i->op] << " " << i->items.at(0)->name;
}

void cisc_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
  // i
}

//
// void one_item_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
//   switch (i->type) {
//     case 0:
//             break;
//
//     case 1:
//             break;
//   }
// }
//
// void two_item_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
//   switch (i->type) {
//     case 0:
//             break;
//
//     case 1:
//             break;
//   }
// }
//
// void cmp_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
//   switch (i->type) {
//     case 0:
//             break;
//
//     case 1:
//             break;
//   }
// }
//
// void cjump_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
//   switch (i->type) {
//     case 0:
//             break;
//
//     case 1:
//             break;
//   }
// }
//
// void CISC_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
//   switch (i->type) {
//     case 0:
//             break;
//
//     case 1:
//             break;
//   }
// }



int main(int argc, char **argv) {
  bool verbose;

  /* Check the input */
  if( argc < 2 ) {
  std::cerr << "Usage: " << argv[ 0 ] << " SOURCE [-v]" << std::endl;
    return 1;
  }
  int32_t opt;
  while ((opt = getopt(argc, argv, "v")) != -1) {
    switch (opt) {
      case 'v':
        verbose = true;
        break;

      default:
        std::cerr << "Usage: " << argv[ 0 ] << "[-v] SOURCE" << std::endl;
        return 1;
    }
  }

  cout << "Init done.\n";
  /* Parse the L1 program.
   */
  L1::Program p = L1::L1_parse_file(argv[optind]);

  // cout << "lalalalalal" << argv[optind] << endl;
  // init .S header
  std::ofstream outputFile;

  // std::string * fileName = new std::string();
  std::string outputFileName;
  outputFileName += "bin/";
  outputFileName += argv[optind];
  outputFileName += ".S";
  // cout << outputFileName;
  // outputFileName = malloc("bin/" + argv[optind] + ".S");
  outputFile.open("prog.S");
  outputFile << ".text\n\t.globl go\ngo:\n\tpushq\t%rbx\n\tpushq\t%rbp\n\tpushq\t%r12\n\tpushq\t%r13\n\tpushq\t%r14\n\tpushq\t%r15\n\n";
  outputFile << "\tcall _" << p.entryPointLabel << "\n\n";
  outputFile << "\tpopq\t%r15\n\tpopq\t%r14\n\tpopq\t%r13\n\tpopq\t%r12\n\tpopq\t%rbp\n\tpopq\t%rbx\n\tretq";
  // outputFile << "_myGo:\n\tmovq $3, %rdi\n\tcall printf\n\tretq";

  /* Generate x86_64 code
   */
  cout << endl << "Program: " << p.entryPointLabel << endl;

  std::map<std::string, std::string> op_map = init_op_map();

  for (auto f : p.functions) {
    outputFile << "\n\n_" << f->name << ":";
    if (f->locals > 0) {
      outputFile << "\n\tsubq $" << std::to_string(f->locals * 8) << ", %rsp";
    }

    for (auto i : f->instructions) {
      switch (i->type) {
        case L1::INS_RETURN: return_ins(& outputFile, i, f);
                break;
        case L1::INS_LABEL: label_ins(& outputFile, i, f);
                break;
        case L1::INS_TWO_OP: L1_ins_two_op(& outputFile, i, f, op_map);
                break;
        case L1::INS_CALL: call_ins(& outputFile, i, f);
                break;
        case L1::INS_GOTO: goto_ins(& outputFile, i, f);
                break;
        case L1::INS_INC_DEC: inc_dec_ins(& outputFile, i, f, op_map);
                break;
        // case 2: two_item_ins(& outputFile, i, f);
        //         break;
        // case 3: cmp_ins(& outputFile, i, f);
        //         break;
        // case 4: cjump_ins(& outputFile, i, f);
        //         break;
        // case 5: CISC_ins(& outputFile, i, f);
        //         break;
      }
    }

    cout << f->name << " has " << f->arguments << " parameters and " << f->locals << " local variables" << endl;
    cout << f->name << " has " << f->instructions.size() << " instructions" << endl;
  }
  outputFile << "\n";
  outputFile.close();



  return 0;
}
