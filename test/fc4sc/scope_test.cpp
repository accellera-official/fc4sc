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
#include <assert.h>

class my_scope : public fc4sc::scope {

public:
SCOPE_DECL(my_scope)

  class fc4sc_covergroup : public covergroup {
  public:
    CG_SCOPED_CONS(fc4sc_covergroup,my_scope) { }


    int x = 0;
    COVERPOINT(int,cvp_int,x) {bin<int>("bin",0)};

  };

  my_scope(std::string name_, const char *auto_inst_file_name = "", int auto_line = 1, fc4sc::global* cntxt = fc4sc::global::getter()) : fc4sc::scope(name_,__FILE__,__LINE__,auto_inst_file_name,auto_line,cntxt), CG_SCOPED_INST(my_cvg)
  { }

  fc4sc_covergroup my_cvg;
};

TEST(scope, test1) {
  auto cntxt = fc4sc::global::create_new_context();
  my_scope mod("my_scope",__FILE__,__LINE__,cntxt);
  xml_printer::coverage_save("scope_test1.xml",cntxt);
  fc4sc::global::delete_context(cntxt);
}

TEST(dynamic_scope, test1) {
  fc4sc::dynamic_scope_factory top_type("top_type"); // type_name = top_type
  fc4sc::dynamic_scope_factory sub_type1(top_type,"sub_type1");//auto sub_type1 = top_type.create_scope("sub_type1"); // type_name = top_type::sub_type1
  fc4sc::dynamic_scope_factory sub_type2("sub_type2"); // type_name = sub_type2
  
  top_type.add_child(sub_type1, "child1");
  top_type.add_child(sub_type2, "child2a");
  top_type.add_child(sub_type2, "child2b");

  fc4sc::dynamic_covergroup_factory cvg(sub_type2,"cvg_type");
  auto cvp = cvg.create_coverpoint<int(int)>("x",[](int x) { return x; });
  cvp.create_bin("ZERO",0);

  sub_type2.add_covergroup(cvg,"cvg");

  auto cntxt = fc4sc::global::create_new_context();

  fc4sc::dynamic_scope top_inst(top_type,"top_inst",__FILE__,__LINE__,cntxt);
  int x_var = 0;
  auto& child2a = top_inst.get_child("child2a").get_covergroup("cvg");
  cvp.bind_sample(child2a,x_var);
  EXPECT_EQ(child2a.get_inst_coverage(),0);
  child2a.sample();
  EXPECT_EQ(child2a.get_inst_coverage(),100);
  
  auto& child2b = top_inst.get_child("child2b").get_covergroup("cvg");
  bool e_catch = false;
  try{
    child2b.sample();
  } catch(std::exception& e) {
    e_catch = true;
  }
  EXPECT_EQ(e_catch,true);
  EXPECT_EQ(child2b.is_enabled(),true);
  EXPECT_EQ(child2b.get_inst_coverage(),0);
  child2b.disable_cvg();
  child2b.sample();
  EXPECT_EQ(child2b.get_inst_coverage(),100);
  cvp.bind_sample(child2b,x_var);
  EXPECT_EQ(child2b.get_inst_coverage(),100);
  child2b.sample();
  EXPECT_EQ(child2b.get_inst_coverage(),100);
  xml_printer::coverage_save("dynamic_scope_test1.xml",cntxt);
  fc4sc::global::delete_context(cntxt);
};

class my_sub_scope : public fc4sc::scope {

public:

SCOPE_DECL(my_sub_scope)  

  class fc4sc_covergroup : public covergroup {
  public:
    CG_SCOPED_CONS(fc4sc_covergroup,my_sub_scope) { }
    int x = 0;
    COVERPOINT(int,cvp_int,x) {bin<int>("ZERO",0)};
  };

  my_sub_scope(fc4sc::scp_base& p_mod, std::string name_, const char *inst_file_name = "", int inst_line = 1) : 
                                     fc4sc::scope(p_mod,name_,__FILE__,__LINE__,inst_file_name,inst_line), 
                                     CG_SCOPED_INST(my_cvg)
				     { }

  fc4sc_covergroup my_cvg;

};


class my_top_scope : public fc4sc::scope {

public:
SCOPE_DECL(my_top_scope)

  class fc4sc_covergroup : public covergroup {
  public:
    CG_SCOPED_CONS(fc4sc_covergroup,my_top_scope) { }

    int x = 0;
    COVERPOINT(int,cvp_int,x) { bin<int>("ZERO",0) };
  };

  my_top_scope(std::string name_, const char *inst_file_name = "", int inst_line = 1,fc4sc::global* cntxt = fc4sc::global::getter()) : fc4sc::scope(name_,__FILE__,__LINE__,inst_file_name,inst_line,cntxt), 
                                     sub_mod(*this,"sub_mod",__FILE__,__LINE__)
				     { }

  std::unique_ptr<fc4sc_covergroup> my_cvg;
  my_sub_scope sub_mod;

};

TEST(scope,test2) {
  auto cntxt = fc4sc::global::create_new_context();
  my_top_scope top("top",__FILE__,__LINE__,cntxt);
  EXPECT_EQ(top.sub_mod.my_cvg.get_inst_coverage(),0);
  top.sub_mod.my_cvg.sample();
  EXPECT_EQ(top.sub_mod.my_cvg.get_inst_coverage(),100);
  xml_printer::coverage_save("scope_test2.xml",cntxt);
  fc4sc::global::delete_context(cntxt);
}

class top : public fc4sc::scope {
public:
  SCOPE_DECL(top)

  class top_cvg : public covergroup {
    public:
    CG_SCOPED_CONS(top_cvg,top) { }
    int x = 0;
    COVERPOINT(int,cvp1,x) { bin<int>("ZERO",0) };
  };

  class child : public fc4sc::scope {
  public:
    SCOPE_DECL(child,top)

    child(fc4sc::scp_base& parent_scp) : fc4sc::scope(parent_scp), CG_SCOPED_INST(top_cvg_inst) { }

    top_cvg top_cvg_inst;

  };

  top(fc4sc::global* cntxt) : fc4sc::scope("top_inst",__FILE__,__LINE__,__FILE__,__LINE__,cntxt) , child_inst(*this) { }

  child child_inst;
};

TEST(toy,toy_scope) {
  auto cntxt = fc4sc::global::create_new_context();
  top inst(cntxt);
  EXPECT_EQ("top",inst.type_name());
  EXPECT_EQ("top::child",inst.child_inst.type_name());
  EXPECT_EQ("top",inst.child_inst.top_cvg_inst.scp_type_name());
  EXPECT_EQ(0,inst.child_inst.top_cvg_inst.get_inst_coverage());
  xml_printer::coverage_save("toy_scope.xml",cntxt);
  fc4sc::global::delete_context(cntxt);
}

TEST(dynamic_scope, demo1) {
  fc4sc::dynamic_scope_factory main("main"); // type_name = top_type
  fc4sc::dynamic_covergroup_factory cg(main,"cg");
  auto flag1_cvp = cg.create_coverpoint<bool(bool)>("flag1_cvp",[](bool x) { return x; });
  flag1_cvp.create_bin("LOW",false);
  flag1_cvp.create_bin("HIGH",true);

  auto flag2_cvp = cg.create_coverpoint<bool(bool)>("flag2_cvp",[](bool x) { return x; });
  flag2_cvp.create_bin("LOW",false);
  flag2_cvp.create_bin("HIGH",true);

  cg.add_cross("flag_crs",flag1_cvp,flag2_cvp);

  main.add_covergroup(cg,"cg_inst");

  auto cntxt = fc4sc::global::create_new_context();

  fc4sc::dynamic_scope main_inst(main,"main",__FILE__,__LINE__,cntxt);
  bool flag1 = false, flag2 = false;
  flag1_cvp.bind_sample(main_inst.get_covergroup("cg_inst"),flag1);
  flag2_cvp.bind_sample(main_inst.get_covergroup("cg_inst"),flag2);
  main_inst.get_covergroup("cg_inst").sample();
  xml_printer::coverage_save("dynamic_scope_demo1.xml",cntxt);
  fc4sc::global::delete_context(cntxt);
}
