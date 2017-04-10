#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include <sstream>
#include <iostream>

#include <L1.h>
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/contrib/raw_string.hh>

using namespace pegtl;
using namespace std;

namespace L1 {

  Item * new_item(std::string str) {
    Item *item = new Item();
    if (str[0] == ':') { // label
      item->type = ITEM_LABEL;
      str.erase(0, 1);
      item->name = str;
    } else if (str[0] == 'r') { // register
      item->type = ITEM_REGISTER;
      item->name = str;
      item->value = -1;
    } else { // number
      item->type = ITEM_NUMBER;
      item->value = std::stoi(str);
    }
    return item;

  }
  //
  Item * new_item2(std::string reg, std::string offset) {
    Item *item = new Item();
    item->type = ITEM_REGISTER;
    item->name = reg;
    item->value = std::stoi(offset);
    return item;
  }

  /*
   * Grammar rules from now on.
   */
  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      pegtl::plus<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};

  struct left_arrow:
    pegtl::string< '<', '-' > {};

  struct plus_minus_op:
    pegtl::sor<
      pegtl::string< '+', '='>,
      pegtl::string< '-', '='>
    > {};

  struct function_name:
    label {};

  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus<
        pegtl::digit
      >
    > {};

  struct argument_number:
    number {};

  struct local_number:
    number {} ;

  struct comment:
    pegtl::disable<
      pegtl::one< ';' >,
      pegtl::until< pegtl::eolf >
    > {};

  struct seps:
    pegtl::star<
      pegtl::sor<
        pegtl::ascii::space,
        comment
      >
    > {};

  struct L1_a:
    pegtl::sor<
      pegtl::string < 'r', 'd', 'i' >,
      pegtl::string < 'r', 's', 'i' >,
      pegtl::string < 'r', 'd', 'x' >,
      pegtl::string < 'r', 'c', 'x' >,
      pegtl::string < 'r', '8' >,
      pegtl::string < 'r', '9' >
    > {};

  struct L1_w:
    pegtl::sor<
      L1_a,
      pegtl::string < 'r', 'a', 'x' >,
      pegtl::string < 'r', 'b', 'x' >,
      pegtl::string < 'r', 'b', 'p' >,
      pegtl::string < 'r', '1', '0' >,
      pegtl::string < 'r', '1', '1' >,
      pegtl::string < 'r', '1', '2' >,
      pegtl::string < 'r', '1', '3' >,
      pegtl::string < 'r', '1', '4' >,
      pegtl::string < 'r', '1', '5' >
    > {};

  struct L1_x:
    pegtl::sor<
      L1_w,
      pegtl::string < 'r', 's', 'p' >
    > {};

  struct L1_s:
    pegtl::sor<
      L1_x,
      number,
      label
    > {};

  struct L1_M:
    number {};

  struct L1_mem_x_M:
    pegtl::seq<
      pegtl::one< '(' >,
      seps,
      pegtl::string < 'm', 'e', 'm' >,
      seps,
      L1_x,
      seps,
      L1_M,
      seps,
      pegtl::one< ')' >,
      seps
    > {};

  struct L1_t:
    pegtl::sor<
      L1_x,
      number
    > {};

  struct L1_u:
    pegtl::sor<
      L1_w,
      label
    > {};

  struct L1_aop:
    pegtl::sor<
      pegtl::string< '+', '=' >,
      pegtl::string< '-', '=' >,
      pegtl::string< '*', '=' >,
      pegtl::string< '&', '=' >
    > {};

  struct L1_sop:
    pegtl::sor<
      pegtl::string< '<', '<', '=' >,
      pegtl::string< '>', '>', '=' >
    > {};

  struct L1_cmp:
    pegtl::sor<
      pegtl::string< '<'>,
      pegtl::string< '<', '=' >,
      pegtl::string< '=' >
    > {};

  struct L1_E:
    pegtl::sor<
      pegtl::one< '0' >,
      pegtl::one< '2' >,
      pegtl::one< '4' >,
      pegtl::one< '8' >
    > {};

  struct L1_system_func:
  pegtl::sor<
  pegtl::string< 'p', 'r', 'i', 'n', 't', ' ', '1'>,
  pegtl::string< 'a', 'l', 'l', 'o', 'c', 'a', 't', 'e', ' ', '2' >,
  pegtl::string< 'a', 'r', 'r', 'a', 'y', '-', 'e', 'r', 'r', 'o', 'r', ' ', '2' >
  > {};

  struct L1_label_rule:
    label {};

  struct L1_ins_two_op:
    pegtl::sor<
      pegtl::seq< L1_w, seps, left_arrow, seps, L1_s >,
      pegtl::seq< L1_w, seps, left_arrow, seps, L1_mem_x_M >,
      pegtl::seq< seps, L1_mem_x_M, seps, left_arrow, seps, L1_s >,
      pegtl::seq< seps, L1_w, seps, L1_aop, seps, L1_t >,
      pegtl::seq< seps, L1_w, seps, L1_sop, seps, pegtl::string< 'r', 'c', 'x' > >,
      pegtl::seq< seps, L1_w, seps, L1_sop, seps, number >,
      pegtl::seq< seps, L1_mem_x_M, seps, plus_minus_op, seps, L1_t >,
      pegtl::seq< seps, L1_w, seps, plus_minus_op, seps, L1_mem_x_M >
    > {};

  struct L1_ins_label:
    label {};

  struct L1_ins_goto:
    pegtl::seq< seps, pegtl::string < 'g', 'o', 't', 'o' >, seps, label, seps > {};

  struct L1_ins_return:
    pegtl::seq< seps, pegtl::string < 'r', 'e', 't', 'u', 'r', 'n' >, seps > {};

  struct L1_ins_call_func:
    pegtl::sor<
      pegtl::seq< seps, pegtl::string < 'c', 'a', 'l', 'l' >, seps, L1_u, seps, number, seps >,
      pegtl::seq< seps, pegtl::string < 'c', 'a', 'l', 'l' >, seps, L1_system_func, seps >
    > {};

  struct L1_ins_inc_dec:
    pegtl::sor<
      pegtl::seq< seps, L1_w, seps, pegtl::string < '+', '+' >, seps >,
      pegtl::seq< seps, L1_w, seps, pegtl::string < '-', '-' >, seps >
    > {};


  struct L1_ins_cisc:
    pegtl::seq< seps, L1_w, seps, pegtl::one< '@' >, seps, L1_w, seps, L1_w, seps, L1_E, seps > {};



  struct L1_instruction:
    pegtl::sor<
      pegtl::seq<
        pegtl::one<'('>,
        pegtl::sor<
          L1_ins_two_op
          // pegtl::seq< seps, L1_w, seps, left_arrow, seps, L1_t_cmp_t >, // ?
          // pegtl::seq< seps, pegtl::string< 'c', 'j', 'u', 'm', 'p' >, seps, L1_t_cmp_t, seps, label, label > //
        >,
        pegtl::one<')'>
      >,
      L1_ins_label,
      pegtl::seq<
        pegtl::one<'('>,
        pegtl::sor<
          L1_ins_goto,
          L1_ins_return,
          L1_ins_call_func,
          L1_ins_inc_dec,
          L1_ins_cisc
        >,
        pegtl::one<')'>
      >
    > {};

  struct L1_function_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      seps,
      pegtl::star<
        L1_instruction,
        seps
      >,
      seps,
      pegtl::one< ')' >,
      seps
    > {};

  struct L1_functions_rule:
    pegtl::seq<
      seps,
      pegtl::plus< L1_function_rule >
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      L1_functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > {};

  struct grammar :
    pegtl::must<
      entry_point_rule
    > {};

  /*
   * Data structures required to parse
   */
  std::vector<L1_item> parsed_registers;


  /*
   * Helper functions
   */
  // vector<std::string> split_by(std::string str, char delimiter) {
  //   vector<std::string> strings;
  //   istringstream f(str);
  //   std::string s;
  //   while (std::getline(f, s, delimiter)) {
  //     cout << s << endl;
  //     strings.push_back(s);
  //   }
  //   return strings;
  // }
  vector<std::string> split_by_space(std::string str) {
    // string str("Split me by whitespaces");
    std::string buf; // Have a buffer string
    std::stringstream ss(str); // Insert the string into a stream
    vector<std::string> tokens; // Create vector to hold our words

    while (ss >> buf)
      tokens.push_back(buf);
      // cout << "haha: " << buf << " " << tokens.size() << endl;
      // cout <<

    return tokens;
  }

  /*
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      if (p.entryPointLabel.empty()) {
        std::string token = in.string();
        token.erase(0, 1);

        p.entryPointLabel = token;
      }
    }
  };

  template<> struct action < function_name > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *newF = new L1::Function();

      std::string token = in.string();
      token.erase(0, 1);

      newF->name = token;
      p.functions.push_back(newF);
    }
  };

  template<> struct action < L1_label_rule > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1_item i;
      i.labelName = in.string();
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < argument_number > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

  template<> struct action < L1_ins_two_op > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      L1::Instruction *newIns = new L1::Instruction();
      newIns->type = L1::INS_TWO_OP;

      vector<std::string> tokens = split_by_space(in.string());

      // cout <<"!!"<< tokens.at(0) << "!!";
      if (tokens.size() == 3) { // no mem assignment
        cout << "tinkering " << tokens.at(0) << " " << tokens.at(1) << " " << tokens.at(2) << endl;
        newIns->items.push_back(L1::new_item(tokens.at(0)));
        newIns->items.push_back(L1::new_item(tokens.at(2)));
        newIns->op = tokens.at(1);

      } else if (tokens.at(0) == "(mem") { // left mem assign
        tokens.at(2).pop_back();
        cout << "tinkering (" << tokens.at(1) << " - " << tokens.at(2) << ") " << tokens.at(3) << " " << tokens.at(4) << endl;
        newIns->items.push_back(L1::new_item2(tokens.at(1), tokens.at(2)));
        newIns->items.push_back(L1::new_item(tokens.at(4)));
        newIns->op = tokens.at(3);
      }
      else { // right mem op
        tokens.at(4).pop_back();
        cout << "tinkering (" << tokens.at(0) << " " << tokens.at(1) << " (" << tokens.at(3) << " - " << tokens.at(4) << ")" << endl;
        newIns->items.push_back(L1::new_item(tokens.at(0)));
        newIns->items.push_back(L1::new_item2(tokens.at(3), tokens.at(4)));
        newIns->op = tokens.at(1);
      }
      currentF->instructions.push_back(newIns);
    }
  };

  template<> struct action < L1_ins_call_func > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      L1::Instruction *newIns = new L1::Instruction();
      newIns->type = L1::INS_CALL;

      vector<std::string> tokens = split_by_space(in.string());

      cout << "tinkering call " << tokens.at(1) << " - " << tokens.at(2) << endl;

      Item *item = new Item();
      item->type = ITEM_REGISTER;
      item->name = tokens.at(1);
      item->value = std::stoi(tokens.at(2));
      newIns->items.push_back(item);

      currentF->instructions.push_back(newIns);
    }
  };

  template<> struct action < L1_ins_return > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      L1::Instruction *newIns = new L1::Instruction();

      cout << "tinkering call return" << endl;

      newIns->type = L1::INS_RETURN;

      currentF->instructions.push_back(newIns);
    }
  };

  template<> struct action < L1_ins_label > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      L1::Instruction *newIns = new L1::Instruction();
      newIns->type = L1::INS_LABEL;

      L1::Item *newItem = new L1::Item();
      newIns->items.push_back(new_item(in.string()));

      cout << "tinkering label: " << in.string() << endl;

      currentF->instructions.push_back(newIns);
    }
  };

  template<> struct action < L1_ins_goto > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      L1::Instruction *newIns = new L1::Instruction();
      newIns->type = L1::INS_GOTO;

      L1::Item *newItem = new L1::Item();
      newIns->items.push_back(new_item(in.string()));

      cout << "tinkering L1_ins_goto: " << in.string() << endl;

      currentF->instructions.push_back(newIns);
    }
  };

  template<> struct action < L1_ins_inc_dec > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      L1::Instruction *newIns = new L1::Instruction();
      newIns->type = L1::INS_INC_DEC;

      std::string insStr = in.string();

      newIns->op = insStr.back();
      insStr.pop_back();
      insStr.pop_back();
      L1::Item *newItem = new L1::Item();
      newIns->items.push_back(new_item(insStr));

      cout << "tinkering L1_ins_inc_dec: " << in.string() << endl;

      currentF->instructions.push_back(newIns);
    }
  };

  template<> struct action < L1_ins_cisc > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      L1::Function *currentF = p.functions.back();
      L1::Instruction *newIns = new L1::Instruction();
      newIns->type = L1::INS_INC_CISC;

      vector<std::string> tokens = split_by_space(in.string());

      L1::Item *newItem = new L1::Item();
      newIns->items.push_back(new_item(tokens.at(0)));
      newIns->items.push_back(new_item(tokens.at(2)));
      newIns->items.push_back(new_item2(tokens.at(3), tokens.at(4)));

      cout << "tinkering L1_ins_cisc: " << in.string() << endl;

      currentF->instructions.push_back(newIns);
    }
  };

  template<> struct action < L1_w > {
    static void apply( const pegtl::input & in, L1::Program & p, L1::Instruction & ti ) {
      cout << "L1_w has been fired.\n";
    }
  };


  Program L1_parse_file (char *fileName) {

    /*
     * Check the grammar for some possible issues.
     */
    // cout << "??\n";
    pegtl::analyze< L1::grammar >();
    // cout << "???\n";
    /*
     * Parse.
     */
    L1::Program p;
    L1::Instruction ti; // temp instruction
    pegtl::file_parser(fileName).parse< L1::grammar, L1::action >(p, ti);

    return p;
  }

} // L1
