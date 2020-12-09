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
#include <climits>
#include <memory>

class my_child_scope : public fc4sc::scope {

public:

SCOPE_DECL(my_child_scope)  

  class fc4sc_covergroup : public covergroup {
  public:
    CG_SCOPED_CONS(fc4sc_covergroup,my_child_scope) { }
    int x = 0;
    COVERPOINT(int,cvp_int,x) {bin<int>("ZERO",0),bin<int>("POSITIVE",interval(1,INT_MAX))};
  };

  my_child_scope(fc4sc::scp_base& p_mod, std::string name_, const char *inst_file_name = "", int inst_line = 1) : 
                                     fc4sc::scope(p_mod,name_,__FILE__,__LINE__,inst_file_name,inst_line), 
                                     CG_SCOPED_INST(my_cvg)
				     { }

  fc4sc_covergroup my_cvg;

};


class my_parent_scope : public fc4sc::scope {

public:
SCOPE_DECL(my_parent_scope)

  class fc4sc_covergroup : public covergroup {
  public:
    CG_SCOPED_CONS(fc4sc_covergroup,my_parent_scope) { }
    int x = 0;
    COVERPOINT(int,cvp_int,x) {bin<int>("ZERO",0),bin<int>("POSITIVE",interval(1,INT_MAX))};
  };

  my_parent_scope(std::string name_, bool cvg, bool child,  const char *inst_file_name = "", int inst_line = 1,fc4sc::global* cntxt = fc4sc::global::getter()) : fc4sc::scope(name_,__FILE__,__LINE__,inst_file_name,inst_line,cntxt) 
  				     {
				       if(cvg) my_cvg = std::unique_ptr<fc4sc_covergroup>(new fc4sc_covergroup(*this,"my_cvg",__FILE__,__LINE__));
				       if(child) sub_mod = std::unique_ptr<my_child_scope>(new my_child_scope(*this,"sub_mod",__FILE__,__LINE__));
				     }

  std::unique_ptr<fc4sc_covergroup> my_cvg;
  std::unique_ptr<my_child_scope> sub_mod;

};

TEST(merge,test1) {
  auto cntxt = fc4sc::global::create_new_context();
  my_parent_scope top("top",false,true,__FILE__,__LINE__,cntxt);
  EXPECT_EQ(top.sub_mod->my_cvg.get_inst_coverage(),0);
  top.sub_mod->my_cvg.sample();
  EXPECT_EQ(top.sub_mod->my_cvg.get_inst_coverage(),50);
  xml_printer::coverage_save("merge_test1.xml",cntxt); 
  fc4sc::global::delete_context(cntxt);
}

TEST(merge,test2) {
  auto cntxt = fc4sc::global::create_new_context();
  my_parent_scope top("top",true,true,__FILE__,__LINE__,cntxt);
  EXPECT_EQ(top.my_cvg->get_inst_coverage(),0);
  top.my_cvg->sample();
  EXPECT_EQ(top.my_cvg->get_inst_coverage(),50);
  xml_printer::coverage_save("merge_test2.xml",cntxt); 
  fc4sc::global::delete_context(cntxt);
}

TEST(merge,test3) {
  auto cntxt = fc4sc::global::create_new_context();
  my_parent_scope top("top",true,false,__FILE__,__LINE__,cntxt); //no child scope in this test
  EXPECT_EQ(top.my_cvg->get_inst_coverage(),0);
  top.my_cvg->x = 1;
  top.my_cvg->sample();
  EXPECT_EQ(top.my_cvg->get_inst_coverage(),50);
  xml_printer::coverage_save("merge_test3.xml",cntxt); 
  fc4sc::global::delete_context(cntxt);
}

TEST(merge,test4) {
  auto cntxt = fc4sc::global::create_new_context();
  my_parent_scope top("top",false,true,__FILE__,__LINE__,cntxt);
  EXPECT_EQ(top.sub_mod->my_cvg.get_inst_coverage(),0);
  top.sub_mod->my_cvg.x = 1;
  top.sub_mod->my_cvg.sample();
  EXPECT_EQ(top.sub_mod->my_cvg.get_inst_coverage(),50);
  xml_printer::coverage_save("merge_test4.xml",cntxt);
  fc4sc::global::delete_context(cntxt);
}
