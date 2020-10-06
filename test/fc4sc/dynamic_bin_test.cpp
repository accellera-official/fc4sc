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

class cvg_dynamic_test : public covergroup {
public:

  CG_CONS(cvg_dynamic_test) {};

  int SAMPLE_POINT(sample_point1, cvp1);
  int SAMPLE_POINT(sample_point2, cvp2);

  void sample(const int& x) {
    this->sample_point1 = x;
    this->sample_point2 = x;
    covergroup::sample();
  }

  coverpoint<int> cvp1 = coverpoint<int> (this, bin<int>("1", 1));
  coverpoint<int> cvp2 = coverpoint<int> (this, bin<int>("2", 1));
};


TEST(dynamic_bin, basic_test) {

  auto cntxt = fc4sc::global::create_new_context();

  cvg_dynamic_test cvg("cvg",__FILE__,__LINE__,cntxt);

  EXPECT_EQ(cvg.get_inst_coverage(), 0);
  
  cvg.stop();
  cvg.sample(1);

  EXPECT_EQ(cvg.get_inst_coverage(), 0);

  cvg.start();
  cvg.cvp1.stop();
  cvg.sample(1);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 0);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);  
  EXPECT_EQ(cvg.get_inst_coverage(), 50);

  cvg.cvp1.start();
  cvg.sample(1);
  
  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);  
  EXPECT_EQ(cvg.get_inst_coverage(), 100);

  bin<int> new_bin = bin<int>("3", 1);
  new_bin.add_to_cvp(cvg.cvp2);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 50);  
  EXPECT_EQ(cvg.get_inst_coverage(), 75);

  cvg.sample(1);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);  
  EXPECT_EQ(cvg.get_inst_coverage(), 100);

  bin<int>("4",2).add_to_cvp(cvg.cvp1);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 50);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);  
  EXPECT_EQ(cvg.get_inst_coverage(), 75);

  cvg.sample(1);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 50);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);  
  EXPECT_EQ(cvg.get_inst_coverage(), 75);

  cvg.sample(2);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);  
  EXPECT_EQ(cvg.get_inst_coverage(), 100);


  xml_printer::coverage_save("dynamic_bin_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

class my_first_cvg : public covergroup {
public:
  int value = 0;
  int flags = 0;
  CG_CONS(my_first_cvg) {};

  COVERPOINT(int, cvp1, value) {
    bin<int>("filled", 1)
  };

  COVERPOINT(int, cvp2, flags) {
    bin<int>("one", 1),
  };

  cross<int,int> cvp_cross = cross<int, int> (this, &cvp1, &cvp2);

};


TEST(dynamic_bin, cross_test) {
  auto cntxt = fc4sc::global::create_new_context();
  my_first_cvg cvg("cvg",__FILE__,__LINE__,cntxt);

  EXPECT_EQ(cvg.get_inst_coverage(), 0);
  
  cvg.stop();
  cvg.value = 1;
  cvg.flags = 1;
  cvg.sample();

  EXPECT_EQ(cvg.get_inst_coverage(), 0);

  cvg.start();
  cvg.cvp1.stop();
  cvg.value = 1;
  cvg.flags = 1;
  cvg.sample();

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 0);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);  
  EXPECT_EQ(cvg.cvp_cross.get_inst_coverage(), 0);  
  EXPECT_EQ((int)cvg.get_inst_coverage(), 33);

  cvg.cvp1.start();
  cvg.value = 1;
  cvg.flags = 1;
  cvg.sample();
  
  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100); 
  EXPECT_EQ(cvg.cvp_cross.get_inst_coverage(), 100);  
  EXPECT_EQ(cvg.get_inst_coverage(), 100);

  bin<int>("dynamic_value_bin",2).add_to_cvp(cvg.cvp1);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 50);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100); 
  EXPECT_EQ(cvg.cvp_cross.get_inst_coverage(), 50);  
  EXPECT_EQ((int)cvg.get_inst_coverage(), 66);


  xml_printer::coverage_save("dynamic_bin_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

TEST(dynamic_covergroup, basic_test) {
  //Dynamic Declaration
  fc4sc::dynamic_covergroup_factory cvg("cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int),bool(int)>("sum",[](int x, int y) {return x+y;},[](int x){return x == 1;});
  cvg_sum.create_bin("ZERO",0);
  cvg_sum.create_bin("ONE",1);

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();
  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"basic_test_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);
  cvg_sum.bind_condition(inst,v2);

  v1 = -1;
  v2 = 1;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),50);

  std::cout << inst.cvg_base::get_coverpoint("sum").get_data() << " cvp of sum\n";

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

TEST(dynamic_covergroup, ignore_bin) {
  //Dynamic Declaration
  fc4sc::dynamic_covergroup_factory cvg("cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int),bool(int)>("sum",[](int x, int y) {return x+y;},[](int x){return x == 1;});
  cvg_sum.create_ignore_bin("ZERO",0);
  cvg_sum.create_bin("ONE",1);

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();

  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"ignore_bin_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);
  cvg_sum.bind_condition(inst,v2);

  v1 = -1;
  v2 = 1;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),0);

  v2 = 2;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),0);

  v1 = 0;
  v2 = 1;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),100);

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

TEST(dynamic_covergroup, illegal_bin) {
  //Dynamic Declaration
  fc4sc::dynamic_covergroup_factory cvg("cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int),bool(int)>("sum",[](int x, int y) {return x+y;},[](int x){return x == 1;});
  cvg_sum.create_illegal_bin("ZERO",0);
  cvg_sum.create_bin("ONE",1);

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();

  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"illegal_bin_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);
  cvg_sum.bind_condition(inst,v2);

  v1 = -1;
  v2 = 1;
  bool e_throw = false;
  try {
    inst.sample();
  } catch(std::exception& e) {
    e_throw = true;
  }
  EXPECT_EQ(e_throw,true);
  EXPECT_EQ(inst.get_inst_coverage(),0);

  v1 = 0;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),100);

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

TEST(dynamic_covergroup, cross_test) {
  //Dynamic Declaration
  fc4sc::dynamic_covergroup_factory cvg("cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int)>("sum",[](int x, int y) {return x+y;});
  cvg_sum.create_bin("ZERO",0);
  cvg_sum.create_bin("ONE",1);

  auto cvg_sum1 = cvg.create_coverpoint<int(int,int)>("sum1",[](int x, int y) {return x+y;});
  cvg_sum1.create_bin("ZERO",0);
  cvg_sum1.create_bin("ONE",1);

  cvg.add_cross("sum_cross",cvg_sum,cvg_sum1);

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();

  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"cross_test_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);
  cvg_sum1.bind_sample(inst,v1,v2);

v1 = -1;
  v2 = 1;
  inst.sample();
  EXPECT_EQ((int)inst.get_inst_coverage(),41);

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

TEST(dynamic_covergroup, bin_array1) {
  //Dynamic Declaration
  fc4sc::dynamic_covergroup_factory cvg("cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int),bool(int)>("sum",[](int x, int y) {return x+y;},[](int x){return x == 1;});
  cvg_sum.create_bin_array("bin_array",5,interval(0,9));

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();

  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"bin_array1_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);
  cvg_sum.bind_condition(inst,v2);

  v1 = -1;
  v2 = 1;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),20);

  v1 = 2;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),40);

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

TEST(dynamic_covergroup, bin_array2) {
  //Dynamic Declaration
  fc4sc::dynamic_covergroup_factory cvg("cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int),bool(int)>("sum",[](int x, int y) {return x+y;},[](int x){return x == 1;});
  cvg_sum.create_bin_array("bin_array",{interval(0,1),interval(2,3),interval(4,5),interval(6,7),interval(8,9)});

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();

  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"bin_array2_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);
  cvg_sum.bind_condition(inst,v2);

  v1 = -1;
  v2 = 1;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),20);

  v1 = 2;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),40);

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}


TEST(dynamic_covergroup, bin_array3) {
  //Dynamic Declaration
  fc4sc::dynamic_covergroup_factory cvg("cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int),bool(int)>("sum",[](int x, int y) {return x+y;},[](int x){return x == 1;});
  std::vector<int> bin_vals{0,1,2,3,4};
  cvg_sum.create_bin_array("bin_array",bin_vals);

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();

  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"bin_array3_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);
  cvg_sum.bind_condition(inst,v2);

  v1 = -1;
  v2 = 1;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),20);

  v1 = 2;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),40);

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

TEST(dynamic_covergroup, scope_test) {
  //Dynamic Declaration
  fc4sc::dynamic_scope_factory main("main");
  fc4sc::dynamic_covergroup_factory cvg(main,"cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int),bool(int)>("sum",[](int x, int y) {return x+y;},[](int x){return x == 1;});
  cvg_sum.create_bin("ZERO",0);
  cvg_sum.create_bin("ONE",1);

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();
  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"basic_test_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);
  cvg_sum.bind_condition(inst,v2);

  v1 = -1;
  v2 = 1;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),50);

  std::cout << inst.cvg_base::get_coverpoint("sum").get_data() << " cvp of sum\n";

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}

TEST(dynamic_covergroup, scope_test_no_condition) {
  //Dynamic Declaration
  fc4sc::dynamic_scope_factory main("main");
  fc4sc::dynamic_covergroup_factory cvg(main,"cvg");
  auto cvg_sum = cvg.create_coverpoint<int(int,int)>("sum",[](int x, int y) {return x+y;});
  cvg_sum.create_bin("ZERO",0);
  cvg_sum.create_bin("ONE",1);

  //Dynamic Instantiation
  auto cntxt = fc4sc::global::create_new_context();
  int v1, v2;
  fc4sc::dynamic_covergroup inst(cvg,"basic_test_inst",__FILE__,__LINE__,cntxt);
  cvg_sum.bind_sample(inst,v1,v2);

  v1 = -1;
  v2 = 1;
  inst.sample();
  EXPECT_EQ(inst.get_inst_coverage(),50);

  std::cout << inst.cvg_base::get_coverpoint("sum").get_data() << " cvp of sum\n";

  xml_printer::coverage_save("dynamic_covergroup_"+std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml",cntxt);
  fc4sc::global::delete_context(cntxt);

}
