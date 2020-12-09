/******************************************************************************

   Copyright 2020 NVIDIA Corporation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

******************************************************************************/
#include "fc4sc.hpp"
#include "gtest/gtest.h"
class example_static_cvg : public covergroup {
public:
  CG_CONS(example_static_cvg) {
    //set options or type options
   }
  bool flag1, flag2;
  COVERPOINT(bool,flag1_cvp,flag1) {
    bin<bool>("LOW",false),
    bin<bool>("HIGH",true)
  };
  COVERPOINT(bool,flag2_cvp,flag2) {
    bin<bool>("LOW",false),
    bin<bool>("HIGH",true)
  };

  cross<bool,bool> flag_crs = cross<bool,bool>(this, "flag_crs", &flag1_cvp, &flag2_cvp);
};


TEST(demo,static_cvg) {
  auto cntxt = fc4sc::global::create_new_context();
  example_static_cvg inst("example_static_cvg_inst",__FILE__,__LINE__,cntxt); 

  inst.flag1 = true;
  inst.flag2 = false;

  inst.sample();
  std::cout << "example_static_cvg coverage = " << inst.get_inst_coverage() << "\n";

  xml_printer::coverage_save("demo.xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

class main : public fc4sc::scope {
public:
SCOPE_DECL(main)
  class cg : public fc4sc::covergroup {
    public:
    CG_SCOPED_CONS(cg,main) { }
    bool flag1, flag2;
    COVERPOINT(bool,flag1_cvp,flag1) {
      bin<bool>("LOW",false),
      bin<bool>("HIGH",true)
    };
    COVERPOINT(bool,flag2_cvp,flag2) {
      bin<bool>("LOW",false),
      bin<bool>("HIGH",true)
    };

    cross<bool,bool> flag_crs = cross<bool,bool>(this, "flag_crs", &flag1_cvp, &flag2_cvp);
  };

  cg cg_inst;

  main(std::string name) : fc4sc::scope(name), CG_SCOPED_INST(cg_inst) { }
};

TEST(demo,scoped_static_cvg) {
  main main_inst("main");
  main_inst.cg_inst.sample();
  std::cout << "coverage = " << main_inst.cg_inst.get_inst_coverage() << "\n";
  xml_printer::coverage_save("scoped_demo.xml");
}
