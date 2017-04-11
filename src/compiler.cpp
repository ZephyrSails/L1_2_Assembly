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

std::map<std::string, std::string> init_cmp_reg_map() {
  std::map<std::string, std::string> cmp_reg_map;
  cmp_reg_map["r8"] = "r8b";
  cmp_reg_map["r9"] = "r9b";
  cmp_reg_map["r10"] = "r10b";
  cmp_reg_map["r11"] = "r11b";
  cmp_reg_map["r12"] = "r12b";
  cmp_reg_map["r13"] = "r13b";
  cmp_reg_map["r14"] = "r14b";
  cmp_reg_map["r15"] = "r15b";
  cmp_reg_map["rax"] = "al";
  cmp_reg_map["rbp"] = "bpl";
  cmp_reg_map["rbx"] = "bl";
  cmp_reg_map["rcx"] = "cl";
  cmp_reg_map["rdi"] = "dil";
  cmp_reg_map["rdx"] = "dl";
  cmp_reg_map["rsi"] = "sil";
  return cmp_reg_map;
}

void return_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
  int offset = f->locals;
  if (f->arguments > 6) {
    offset += f->arguments - 6;
  }
  if (offset > 0) {
    *outputFile << "\n\taddq $" << std::to_string(offset * 8) << ", %rsp";
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

void mem_or_w_start_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f, std::map<std::string, std::string> op_map) {
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
  // item->name.erase(0, 1);
  item->name.insert(0, "_");
  *outputFile << "\n\tjmp " << item->name;
}

void inc_dec_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f, std::map<std::string, std::string> op_map) {
  *outputFile << "\n\t" << op_map[i->op] << " $" << i->items.at(0)->name;
}

void cisc_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f) {
  *outputFile << "\n\tlea (%" << i->items.at(1)->name << ", %" << i->items.at(2)->name << ", " << std::to_string(i->items.at(2)->value) << "), %" << i->items.at(0)->name;
  // i
}

void cmp_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f, std::map<std::string, std::string> cmp_reg_map) {
  L1::Item *dest = i->items.at(0);

  L1::Item *cmpl = i->items.at(1);
  std::string op = i->op;
  L1::Item *cmpr = i->items.at(2);
  if (cmpl->type == L1::ITEM_NUMBER && cmpr->type == L1::ITEM_NUMBER) {
    std::string res;
    if (op == "<") {
      res = cmpl->value < cmpr->value ? "1" : "0";
    } else if (op == "<=") {
      res = cmpl->value <= cmpr->value ? "1" : "0";
    } else { // if (op == "=")
      res = cmpl->value == cmpr->value ? "1" : "0";
    }
    *outputFile << "\n\tmovq $" << res << ", %" << dest->name;
  } else if (cmpl->type == L1::ITEM_NUMBER) { // do something, because cmpl must be a register
    std::string setop;
    if (op == "<") {
      setop = "g";
    } else if (op == "<=") {
      setop = "ge";
    } else { // if (op == "=")
      setop = "e";
    }
    *outputFile << "\n\tcmpq $" << std::to_string(cmpl->value) << ", %" << cmpr->name;
    *outputFile << "\n\tset" << setop << " %" << cmp_reg_map[dest->name];
    *outputFile << "\n\tmovzbq %" << cmp_reg_map[dest->name] << ", %" << dest->name;
  } else { // just do the normal stuff
    std::string setop;
    if (op == "<") {
      setop = "l";
    } else if (op == "<=") {
      setop = "le";
    } else { // if (op == "=")
      setop = "e";
    }
    std::string cmpql;
    if (cmpr->type == L1::ITEM_NUMBER) {
      cmpql = "$" + std::to_string(cmpr->value);
    } else {
      cmpql = "%" + cmpr->name;
    }
    *outputFile << "\n\tcmpq " << cmpql << ", %" << cmpl->name;
    *outputFile << "\n\tset" << setop << " %" << cmp_reg_map[dest->name];
    *outputFile << "\n\tmovzbq %" << cmp_reg_map[dest->name] << ", %" << dest->name;
  }
}

void cjump_ins(std::ofstream * outputFile, L1::Instruction * i, L1::Function * f, std::map<std::string, std::string> cmp_reg_map) {
  L1::Item *cmpl = i->items.at(0);
  std::string op = i->op;
  L1::Item *cmpr = i->items.at(1);
  std::string labelT = i->items.at(2)->name;
  std::string labelF = i->items.at(3)->name;

  if (cmpl->type == L1::ITEM_NUMBER && cmpr->type == L1::ITEM_NUMBER) {
    std::string cjumpDest;
    if (op == "<") {
      cjumpDest = cmpl->value < cmpr->value ? labelT : labelF;
    } else if (op == "<=") {
      cjumpDest = cmpl->value <= cmpr->value ? labelT : labelF;
    } else { // if (op == "=")
      cjumpDest = cmpl->value == cmpr->value ? labelT : labelF;
    }
    *outputFile << "\n\tjmp _" << cjumpDest;
  } else if (cmpl->type == L1::ITEM_NUMBER) { // do something, because cmpl must be a register
    std::string jop;
    if (op == "<") {
      jop = "g";
    } else if (op == "<=") {
      jop = "ge";
    } else { // if (op == "=")
      jop = "e";
    }
    *outputFile << "\n\tcmpq $" << std::to_string(cmpl->value) << ", %" << cmpr->name;
    *outputFile << "\n\tj" << jop << " _" << labelT;
    *outputFile << "\n\tjmp _" << labelF;
  } else { // just do the normal stuff
    std::string jop;
    if (op == "<") {
      jop = "l";
    } else if (op == "<=") {
      jop = "le";
    } else { // if (op == "=")
      jop = "e";
    }
    std::string cmpql;
    if (cmpr->type == L1::ITEM_NUMBER) {
      cmpql = "$" + std::to_string(cmpr->value);
    } else {
      cmpql = "%" + cmpr->name;
    }
    *outputFile << "\n\tcmpq " << cmpql << ", %" << cmpl->name;
    *outputFile << "\n\tj" << jop << " _" << labelT;
    *outputFile << "\n\tjmp _" << labelF;
  }
}

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

  // cout << "Init done.\n";
  /* Parse the L1 program.
   */
  L1::Program p = L1::L1_parse_file(argv[optind]);

  // cout << "lalalalalal" << argv[optind] << endl;
  // init .S header
  std::ofstream outputFile;

  // std::string * fileName = new std::string();
  // std::string outputFileName;
  // outputFileName += "bin/";
  // outputFileName += argv[optind];
  // outputFileName += ".S";
  // cout << outputFileName;
  // outputFileName = malloc("bin/" + argv[optind] + ".S");
  outputFile.open("prog.S");
  outputFile << ".text\n\t.globl go\ngo:\n\tpushq\t%rbx\n\tpushq\t%rbp\n\tpushq\t%r12\n\tpushq\t%r13\n\tpushq\t%r14\n\tpushq\t%r15\n\n\tcall _" << p.entryPointLabel << "\n\n\tpopq\t%r15\n\tpopq\t%r14\n\tpopq\t%r13\n\tpopq\t%r12\n\tpopq\t%rbp\n\tpopq\t%rbx\n\tretq";

  /* Generate x86_64 code
   */
  // cout << endl << "Program: " << p.entryPointLabel << endl;
  cout << argv[optind] << endl;

  std::map<std::string, std::string> op_map = init_op_map();
  std::map<std::string, std::string> cmp_reg_map = init_cmp_reg_map();

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
        case L1::INS_MEM_OR_W_START: mem_or_w_start_ins(& outputFile, i, f, op_map);
                break;
        case L1::INS_CALL: call_ins(& outputFile, i, f);
                break;
        case L1::INS_GOTO: goto_ins(& outputFile, i, f);
                break;
        case L1::INS_INC_DEC: inc_dec_ins(& outputFile, i, f, op_map);
                break;
        case L1::INS_CISC: cisc_ins(& outputFile, i, f);
                break;
        case L1::INS_CMP: cmp_ins(& outputFile, i, f, cmp_reg_map);
                break;
        case L1::INS_CJUMP: cjump_ins(& outputFile, i, f, cmp_reg_map);
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

    // cout << f->name << " has " << f->arguments << " parameters and " << f->locals << " local variables" << endl;
    // cout << f->name << " has " << f->instructions.size() << " instructions" << endl;
  }
  outputFile << "\n";
  outputFile.close();

  return 0;
}
