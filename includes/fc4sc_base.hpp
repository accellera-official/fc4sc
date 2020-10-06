/******************************************************************************

   Copyright 2003-2018 AMIQ Consulting s.r.l.
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
/******************************************************************************
   Original Authors: Teodor Vasilache and Dragos Dospinescu,
                     AMIQ Consulting s.r.l. (contributors@amiq.com)

               Date: 2018-Feb-20
******************************************************************************/

/*!
 \file fc_base.hpp
 \brief Helper functions and interfaces

 This file contains helper functions and the base classes
 for bins, coverpoints and covergroups.
 */

#ifndef FC4SC_BASE_HPP
#define FC4SC_BASE_HPP

#include <map>
#include <iostream>
#include <string>
#include <exception>
#include <algorithm>
#include <tuple>
#include <vector>
#include <utility>
#include <functional>
#include <unordered_map>
#include <assert.h>
#include <memory>

#include "fc4sc_options.hpp"

/*
 * Template meta-programming tool used for checking that a parameter
 * pack doesn't contain any argument which is convertible to a specified type.
 * This is used in the bin & coverpoint constructors together with a
 * static_assert in order to make sure that the user does not provide
 * multiple arguments which must be provided only once.
 * For bin -> the name
 * For coverpoint -> pointer to the parent covergroup
 */
template<typename forbidden, typename head, typename...tail> struct forbid_type {
  static bool constexpr value = forbid_type<forbidden, tail...>::value &&
    (!std::is_convertible<head, forbidden>::value);
};
template<typename forbidden, typename first> struct forbid_type<forbidden, first> :
  public std::__not_<std::is_convertible<first, forbidden>> {};

/*!
 * \brief Macro to declare constructor for covergroup registration.
 *
 * This macro expands to a constructor declaration for your custom type, which calls a parent constructor
 * to register the new instance, with some additional info useful for displaying the final report.
 */
#define CG_CONS_DECL(type, args...) \
  using covergroup::sample; \
  type(std::string inst_name, const char *auto_inst_file_name, int auto_line, fc4sc::global* cntxt, ##args)

/*!
 * \brief Macro to define constructor for covergroup registration.
 *
 * This macro expands to a constructor definition for your custom type, which calls a parent constructor
 * to register the new instance, with some additional info useful for displaying the final report.
 */
#define CG_CONS_DEF(type, args...) \
  type::type(std::string inst_name = "", const char *auto_inst_file_name = "", int auto_line = 1, fc4sc::global* cntxt = fc4sc::global::getter(), ##args) : fc4sc::covergroup(#type, __FILE__, __LINE__, inst_name, auto_inst_file_name, auto_line, cntxt)

/*!
 * \brief Macro to register your covergroup.
 *
 * This macro expands to a constructor for your custom type, which calls a parent constructor
 * to register the new instance, with some additional info useful for displaying the final report.
 */
#define CG_CONS(type, args...) \
  using covergroup::sample; \
  type(std::string inst_name = "", const char *auto_inst_file_name = "", int auto_line = 1, fc4sc::global* cntxt = fc4sc::global::getter(), ##args) : fc4sc::covergroup(#type, __FILE__, __LINE__, inst_name, auto_inst_file_name, auto_line, cntxt)

/*!
 * \brief Macro to declare constructor for covergroup registration within a scope.
 *
 * This macro expands to a constructor declaration for your custom type, which calls a parent constructor
 * to register the new instance, with some additional info useful for displaying the final report.
 * Requires that the covergroup must be associated with a scope when instantiated
 */
#define CG_SCOPED_CONS_DECL(type, scp_type, args...) \
  using covergroup::sample; \
  type(fc4sc::scp_base& scp, std::string inst_name, const char *auto_inst_file_name, int auto_line, ##args)

/*!
 * \brief Macro to define constructor for covergroup registration within a scope.
 *
 * This macro expands to a constructor definition for your custom type, which calls a parent constructor
 * to register the new instance, with some additional info useful for displaying the final report.
 * Requires that the covergroup must be associated with a scope when instantiated
 */
#define CG_SCOPED_CONS_DEF(type, scp_type, args...) \
  scp_type::type::type(fc4sc::scp_base& scp, std::string inst_name = "", const char *auto_inst_file_name = "", int auto_line = 1, ##args) : fc4sc::covergroup(scp, #type, scp_type::scp_type_name(), __FILE__, __LINE__, inst_name, auto_inst_file_name, auto_line)


/*!
 * \brief Macro to register your covergroup within a scope.
 *
 * This macro expands to a constructor for your custom type, which calls a parent constructor
 * to register the new instance, with some additional info useful for displaying the final report.
 * Requires that the covergroup must be associated with a scope when instantiated.
 */
#define CG_SCOPED_CONS(type, scp_type, args...) \
  using covergroup::sample; \
  type(fc4sc::scp_base& scp, std::string inst_name = "", const char *auto_inst_file_name = "", int auto_line = 1, ##args) : fc4sc::covergroup(scp, #type, scp_type::scp_type_name(), __FILE__, __LINE__, inst_name, auto_inst_file_name, auto_line)

/*!
 * \brief Macro to register your covergroup within a scope.
 *
 * This macro expands to a constructor for your custom type, which calls a parent constructor
 * to register the new instance, with some additional info useful for displaying the final report.
 * Requires that the covergroup must be associated with a scope when instantiated.
 */
#define CG_SCOPED_INST(name, args...) \
  name(*this,#name,__FILE__,__LINE__,##args)

/*!
 * \brief Macro to declare a sampling point variable and register some names
 */
#define SAMPLE_POINT(variable_name, cvp) variable_name = static_cast<decltype(variable_name)> \
  (covergroup::set_strings(&cvp, &variable_name, #cvp, #variable_name));

/*
 * \brief Macro that creates a lambda function which returns the expression given
 * as argument when called. A pointer to the enclosing class will be bound in the
 * lambda capture so that local class variables can be used in the lambda body.
 * The purpose of this macro is to use in the instantiation of coverpoints inside
 * covergroups and will serve as a generator for lambda expressions to be used
 * when sampling (sample expression and sample conditions).
 */
#define CREATE_WRAP_F(expr, type) [&]() -> type { return (expr); }

#define GET_CVP_MACRO(_1,_2,_3,_4, NAME,...) NAME

/*
 * Macro used to define and automatically register a coverpoint inside a covergroup.
 * The macro accepts 3 or 4 arguments in the following order:
 * 1) Type: used for the values of the bins declared in the coverpoint
 * 2) Name: the name of the coverpoint
 * 3) Sample Expression: the expression that will be used for sampling this coverpoint
 * whenever the covergroup is sampled (the type has to be the same as Type argument)
 * 4) Sample Condition: a condition that has to be true in order for this coverpoint
 * to be sampled when the covergroup is sampled (the type has to be bool)
 */
#define COVERPOINT(...) GET_CVP_MACRO(__VA_ARGS__, CVP_4, CVP_3)(__VA_ARGS__)

// COVERPOINT macro for 3 arguments (no sample condition)
#define CVP_3(type, cvp_name, sample_expr) \
        coverpoint<type> cvp_name = \
        covergroup::register_cvp<type>(&cvp_name, #cvp_name, \
        CREATE_WRAP_F(sample_expr, type), #sample_expr, \
        CREATE_WRAP_F(true, bool), std::string("")) =

// COVERPOINT macro for 4 arguments (sample condition included)
#define CVP_4(type, cvp_name, sample_expr, sample_cond) \
        coverpoint<type> cvp_name = \
        covergroup::register_cvp<type>(&cvp_name, #cvp_name, \
        CREATE_WRAP_F(sample_expr, type), #sample_expr, \
        CREATE_WRAP_F(sample_cond, bool), #sample_cond) =

// global var for type name and instance name of default scopes
// covergroups that are not associated with user-defined scopes automatically are associated with the default scope
#define _FC4SC_DEFAULT_SCOPE_TYPE_ "default_scope"
#define _FC4SC_DEFAULT_SCOPE_NAME_ "default_scope_instance"

/*!
 *  \namespace fc4sc
 *  \brief Namespace containing the library
 */
namespace fc4sc
{
using cvp_metadata_t = std::tuple<void*, std::string, std::string>;

class covVisitorBase;
class covergroup;
class cvg_base;
class scp_base;
class cross_base;
class coverpoint_base;
class bin_base;
class dynamic_covergroup_factory;
class scp_base_data_model;
class cvg_base_data_model;
class coverpoint_base_data_model;
class cross_base_data_model;
class bin_base_data_model;
class global;
class cvg_metadata;
class scp_metadata;
/*!
 *  \brief Alias to std::make_pair
 *  \param t1 One end of the interval
 *  \param t2 Other end of the interval
 *  \returns Pair of t1 and t2
 *
 *    Just a wrapper over make_pair to declare value intervals
 *  in a more verbose way.
 */
template <typename T>
static interval_t<T> interval(T t1, T t2) {
  return interval_t<T>(t1, t2);
}

/*!
 * \brief Define a type for bin types
 */
typedef enum
{
  default_,
  illegal_,
  ignore_
} bin_t;

/*!
 *  \class covVisitorBase fc_base.hpp
 *  \brief Base class for coverage model visitor
 *
 *  Basic interface to introspect the model via Visitor Pattern.
 *  This class is intended to be extended by the user
 */
class covVisitorBase {
public:
 
  /*! visit a scope's data model */
  virtual void visit(scp_base_data_model&) = 0;

  /*! visit a covergroup's data model */
  virtual void visit(cvg_base_data_model&) = 0;

  /*! visit a coverpoint's data model */
  virtual void visit(coverpoint_base_data_model&) = 0;

  /*! visit a cross's data model */
  virtual void visit(cross_base_data_model&) = 0;

  /*! visit a bin's data model */
  virtual void visit(bin_base_data_model&) = 0;

};

/*!
 *  \class bin_base_data_model fc_base.hpp
 *  \brief Base class for bin_data_model class
 *
 *  Base class with getters to coverage data objects corresponding to bins.
 *  The purpose of the getters is for coverage model introspection.
 */
class bin_base_data_model {
public:
 
  /*! Indicates if the data within this object is valid */
  std::shared_ptr<bool> valid = std::make_shared<bool>(true);

  /*! Get intervals in type int */
  virtual std::vector<interval_t<int>> get_intervals_to_int() const = 0;

  /*! Get name of bin */
  virtual std::string& get_name() = 0;

  /*! Get bin type */
  virtual bin_t& get_bin_type() = 0;

  /*! Get hit count for each interval in bin */
  virtual std::vector<uint64_t>& get_interval_hits() = 0;

  /*! Visitor Pattern for introspection */
  virtual void accept_visitor(covVisitorBase& visitor) = 0;

  /*! Destructor */
  virtual ~bin_base_data_model() { }

};

/*!
 *  \class bin_base fc_base.hpp
 *  \brief Base class for bins
 *
 *  Basic interface to work with untemplated bins
 */
class bin_base
{
public:
  /*!
   * \brief Returns total number of hits
   * \returns number of times the sampled value was in the bin
   */
  virtual uint64_t get_hitcount() const = 0;
 
  /*! Get intervals casted as int type */
  virtual std::vector<interval_t<int>> get_intervals_to_int() const = 0;

  /*! Get reference to bin name */
  virtual std::string& name() = 0;

  /*! Get reference to bin type */
  virtual bin_t& bin_type() = 0;

  /*! Get reference to bin counts */
  virtual std::vector<uint64_t>& interval_hits() = 0;

  /*! Destructor */
  virtual ~bin_base(){}
};

/*!
 *  \class api_base_data_model fc_base.hpp
 *  \brief Base class for api_data_model class
 *
 *  Base class that contains the name of coverage object and valid ptr.
 */
class api_data_model {
public:

  /*! Valid pointer for garbage collection */
  std::shared_ptr<bool> valid = std::make_shared<bool>(true);

  /*! Coverage object name */
  std::string name;

};

/*!
 *  \class bin_base fc_base.hpp
 *  \brief Base class for bins
 *
 *  Basic interface for coverage objects that implements coverage calculation functions
 */
class api_base
{
public:

  /*! Get reference to coverage object name */
  virtual std::string& name() = 0;

  /** \name Coverage API
   *  API for getting and controlling coverage collection at run time
   */
  /**@{*/

  /*!
   * \brief Returns the coverage associated with this instance
   * \returns Double between 0 and 100
   */
  virtual double get_inst_coverage() const = 0;

  /*!
   * \brief Returns the coverage associated with this instance
   * \param hit Number of hit bins in this instance
   * \param total Total number of bins in this instance
   * \returns Double between 0 and 100
   */
  virtual double get_inst_coverage(int &hit, int &total) const = 0;

  /*!
   * \brief Enables sampling on this instance
   */
  virtual void start() = 0;

  /*!
   * \brief Stops sampling on this instance
   */
  virtual void stop() = 0;
  /**@}*/

  /*! Sample coverage object */
  virtual void sample() = 0;

  /*! Destructor */
  virtual ~api_base(){}
};

/*!
 *  \class cvp_base_data_model fc_base.hpp
 *  \brief Base class for coverpoints and crosses
 *
 *  Basic interface to work with untemplated coverpoints and crosses
 */
class cvp_base_data_model : public api_data_model
{
public:

  /*! Number of sample misses (no bin hit)*/
  uint64_t misses = 0;

  /*! Visitor function for introspection */
  virtual void accept_visitor(covVisitorBase&) = 0;

  /*! Get coverpoint size */
  virtual uint64_t size() const = 0;

  /*! Destructor */
  virtual ~cvp_base_data_model() { }

};

/*!
 *  \class cvp_base fc_base.hpp
 *  \brief Base class for coverpoints and crosses
 *
 *  Basic interface to work with untemplated coverpoints and crosses
 */
class cvp_base : public api_base
{
protected:
  bool stop_sample_on_first_bin_hit = false;

public:
  size_t last_bin_index_hit = 0;
  bool last_sample_success = false;

  virtual cvp_base* create_instance(cvg_base* cvg_inst) = 0;

  /*! Instance specific options */
  virtual uint64_t get_weight() = 0;

  /*! Get coverpoint size */
  virtual uint64_t size() const = 0;

  /*! Get total bin misses */
  virtual uint64_t get_misses() = 0;

  /*! Get pointer to data model */
  virtual cvp_base_data_model* get_data() = 0;

  /*! Destructor */
  virtual ~cvp_base(){}
};

/*!
 *  \class coverpoint_base_data_model fc_base.hpp
 *  \brief Base class for data of coverpoints
 *
 *  Basic interface to work with data of coverpoints
 */
class coverpoint_base_data_model : public cvp_base_data_model
{
public:

  /*! Coverpoint options */
  cvp_option option;

  /*! Vector of pointers to regular bin data */
  std::vector<bin_base_data_model*> bins_data;

  /*! Vector of pointers to illegal bin data */
  std::vector<bin_base_data_model*> illegal_bins_data;

  /*! Vector of pointers to ignore bin data */
  std::vector<bin_base_data_model*> ignore_bins_data;

  /*! Get reference to sample expression string */
  virtual std::string& get_sample_expression_str() = 0;

  /*! Get reference to sample condition string */
  virtual std::string& get_sample_condition_str() = 0;

  /*! Destructor */
  virtual ~coverpoint_base_data_model()
  {
    for(auto bin_it : this->bins_data) {
      delete bin_it;
    }
    for(auto bin_it : this->illegal_bins_data) {
      delete bin_it;
    }
    for(auto bin_it : this->ignore_bins_data) {
      delete bin_it;
    }
  }

};

/*!
 *  \class coverpoint_base
 *  \brief non-template coverpoint class
 *
 *  Interface to introspect the coverage model
 */
class coverpoint_base : public cvp_base
{
public:
  
  /*! Get reference to coverpoint options */
  virtual cvp_option& option() = 0;

  /*! Get reference to sample_expression string */
  virtual std::string& get_sample_expression_str() = 0;

  /*! Get vector of regular bins associated with this coverpoint */
  virtual std::vector<bin_base*> get_bins_base() = 0;

  /*! Get vector of illegal bins associated with this coverpoint */
  virtual std::vector<bin_base*> get_illegal_bins_base() = 0;

  /*! Get vector of ignore bins associated with this coverpoint */
  virtual std::vector<bin_base*> get_ignore_bins_base() = 0;

  /*! Destructor */
  virtual ~coverpoint_base() { }

};

/*!
 *  \class cross_base_data_model fc_base.hpp
 *  \brief Base class for data of crosses
 *
 *  Basic interface to work with data of crosses
 */
class cross_base_data_model : public cvp_base_data_model
{
public:

  /*! Cross options */
  cross_option option;

  /*! Vector of crossed coverpoints */
  std::vector<cvp_base_data_model*> cross_cvps;

  /*! Get cross bins */
  virtual const std::map<std::vector<size_t>, uint64_t>& get_cross_bins() const = 0;

};

/*!
 *  \class cross_base
 *  \brief non-template cross class
 *
 *  Interface to introspect the coverage model
 */
class cross_base : public cvp_base
{
public:

  /*! Get reference to cross options */
  virtual cross_option& option() = 0;

  /*! Get cross bins storage */
  virtual const std::map<std::vector<size_t>, uint64_t>& get_cross_bins() const = 0;

  /*! Get crossed coverpoints storage */
  virtual const std::vector<cvp_base *>& get_cross_coverpoints() const = 0;

  /*! Destructor */
  virtual ~cross_base() { }
};

/*!
 *  \class cvg_base_data_model fc_base.hpp
 *  \brief Base class for data of crosses
 *
 *  Basic interface to work with data of covergroups
 */
class cvg_base_data_model : public api_data_model
{

friend class covergroup;
friend class dynamic_covergroup;

public:

  /*! Pointer for type data of the covergroup */
  cvg_metadata* type_data;

  /*! Indicator for optional covergroup */
  bool enable = true;

  /*! Vector of pointers to coverpoints/crosses of the covergroup */
  std::vector<cvp_base_data_model*> cvps;

  /*! Instance specific options */
  cvg_option option;
  
  /*! File ID associated with the instance */
  unsigned int inst_file_id;

  /*! Aproximate line where this instance is instantiated */
  uint32_t inst_line;

  /*! Pointer to data of parent scope instance */
  scp_base_data_model* parent_scp = nullptr;


  /*! Add coverpoint/cross data to the covergroup */
  void add_cvp_data(cvp_base_data_model* cvp_data)
  {
    cvps.push_back(cvp_data);
  }
  
  /*!
   * \brief Visitor pattern for coverage model introspection
   */
  virtual void accept_visitor(covVisitorBase& visitor)
  {
    visitor.visit(*this);
  }

  /*! Destructor */
  virtual ~cvg_base_data_model()
  {
    for(auto cvp_it : cvps) {
      delete cvp_it;
    }
  }

};

/*!
 *  \class cvg_base fc_base.hpp
 *  \brief Base class for covergroups
 *
 *  Basic interface to work with covergroups
 */
class cvg_base : public api_base
{
public:

  /*! get coverpoint reference by name within this covergroup*/ 
  cvp_base& get_coverpoint(std::string name)
  {
    for (auto& cvp : cvps)
    {
      if(name == cvp->name()) {
        return *cvp;
      }
    }
    throw("No coverpoint " + name + " in " + this->name());
  }

  /*! Get pointer to covergroup data */
  virtual cvg_base_data_model* get_cvg_data() = 0;

  /*! Indictor if covergorup is enabled */
  virtual bool is_enabled() const = 0;

  /*! Register coverpoint to covergroup */
  virtual void register_cvp(cvp_base* const cvp) {
    // Runtime check to make sure that all the coverpoints are different.
    // Normally, this should be the case, but if for whatever reason this
    // function is used multiple times with pointers to the the same coverpoint,
    // we make sure that the damage is minimal at least.
    if (std::find(std::begin(cvps), std::end(cvps), cvp) != std::end(cvps))
      throw ("Coverpoint already registered in this covergroup!");
    cvps.push_back(cvp);
  }

  /*! Disable covergroup */
  virtual void disable_cvg() = 0;

  /*! Retrieve sampling point, instance name and sampling point name */ 
  virtual cvp_metadata_t get_strings(cvp_base *cvp) = 0;

  /*! Array of associated coverpoints and crosses */
  std::vector<cvp_base *> cvps;
  
  /*! Instance specific options */
  virtual cvg_option& option() const = 0;

  /*! Type specific options */
  virtual cvg_type_option& type_option() = 0;

  /*! Name of the covergroup type */
  virtual std::string& type_name() = 0; 

  /*! File ID associated with where this type is declared */
  virtual unsigned int& file_id() = 0;

  /*! Aproximate line where this type is declared */
  virtual uint32_t& line() = 0;

  /*! File ID associated with the instance */
  virtual unsigned int& inst_file_id() = 0;

  /*! Aproximate line where this instance is instantiated */
  virtual uint32_t& inst_line() = 0;

  /*!
   * \brief pointer to parent scope that contains this covergroups. Note can be a nullptr
   * since covergroups are not required to be contained within scopes
   */
  fc4sc::scp_base* parent_scp = nullptr;

};

/*!
 *  \class scope_factory_base fc_base.hpp
 *  \brief Base class for scopes
 *
 *  Basic interface to work with dynamic_scope_factory
 */
class scope_factory_base {

public:

  /*! Register covergroup type to factory */
  virtual void register_sub_cvg_type(dynamic_covergroup_factory* cvg) = 0;

  /*! Get the type name this factory represents */
  virtual std::string get_scp_type_name() = 0;

};

/*!
 *  \class cvg_base_data_model fc_base.hpp
 *  \brief Base class for data of crosses
 *
 *  Basic interface to work with data of covergroups
 */
class scp_base_data_model {
public:


  /*! Valid ptr for garbage collection */
  std::shared_ptr<bool> valid = std::make_shared<bool>(true);

  /*! Counter for coverage objects without names instanced in this scope*/
  int anonymous_count = 0;

  /*! Scope instance name */
  std::string name;

  /*! File ID associated with the instance */
  unsigned int inst_file_id;

  /*! Aproximate line where this instance is instantiated */
  uint32_t inst_line;

  /*! Scope instance id */
  unsigned int instance_id;

  /*! Pointer to parent scope data */
  scp_base_data_model* parent_scp = nullptr;

  /*! Pointer to context this scope belongs */
  global* cntxt = nullptr;

  /*! Pointer to type data of this scope */
  scp_metadata* type_data;

  /*! Map from instance name to scope data pointer for sub scopes instanced in this scope */
  std::unordered_map<std::string,scp_base_data_model*> child_scp_insts;

  /*! Map from type name to Vector of scope data pointer for sub scopes instanced in this scope*/
  std::unordered_map<std::string,std::vector<scp_base_data_model*>> child_scps;

  /*! Map from instance name to covergroup data pointer for covergroups instanced in this scope */
  std::unordered_map<std::string,cvg_base_data_model*> cvg_insts;

  /*! Map from type name to Vector of covergroup data pointer for covergroups instanced in this scope */
  std::unordered_map<std::string,std::vector<cvg_base_data_model*>> cvgs;

  /*! Add covergroup instance to this scope */
  virtual void add_cvg_data(cvg_base_data_model* cvg_data,std::string cvg_type_name, std::string scp_type_name, unsigned int cvg_file_id, uint32_t cvg_line) = 0;

  /*! Add scope instance to this scope */
  virtual void add_scp_data(scp_base_data_model* scp_data, std::string scp_type_name, unsigned int scp_file_id, uint32_t scp_line) = 0;

  /*! Checks for instances names already being used in this scope instance */
  void name_check(std::string& name, std::string type_name)
  {
    if (name.empty())
    {
      std::size_t found = type_name.find_last_of(':');    
      if(found == std::string::npos) {
	name = type_name + "_" + std::to_string(anonymous_count++);
      }
      else {
        name = type_name.substr(found+1) + "_" + std::to_string(anonymous_count++);
      }
    }
    if(cvg_insts.count(name) > 0 || child_scp_insts.count(name) > 0)
    {
      std::cerr << name << " scope name already in use\n";
      throw;
    }
  }

  /*!
   * \brief Visitor pattern for coverage model introspection
   */
  virtual void accept_visitor(covVisitorBase& visitor)
  {
    visitor.visit(*this);
  }

  /*! Destructor */
  virtual ~scp_base_data_model() { }

};

/*!
 *  \class scp_base fc_base.hpp
 *  \brief Base class for scopes
 *
 *  Basic interface to work with scopes
 */
class scp_base {
public:

  /*! Register covergroup to this scope */
  void register_cvg(cvg_base* cvg)
  {
    cvg->parent_scp = this;
    cvg_insts[cvg->name()] = cvg;
  }

  /*! Register scope instance to this scope */
  void register_child_scope(scp_base* const scp)
  {
    child_scp_insts[scp->name()] = scp;
  }

  /*! Get sub/child scope instance by name */
  scp_base& get_child(std::string name)
  {
    if(child_scp_insts.count(name) > 0) {
      return *(child_scp_insts[name]);
    }
    throw("No child scope " + name + " in " + this->name());
  }

  /*! Get covergroup instance by name */
  cvg_base& get_covergroup(std::string name)
  {
    if(this->cvg_insts.count(name) > 0) {
      return *(this->cvg_insts[name]);
    }
    throw("No covergroup " + name + " in " + this->name());
  }


  /*! Get pointer to scope's data */
  virtual scp_base_data_model* get_scp_data() = 0;
  
  /*! Name of scope instance */
  virtual std::string& name() = 0;

  /*! Name of the scope type */
  virtual std::string type_name() = 0; 

  /*! File ID associated with where this type is declared */
  virtual unsigned int& file_id() = 0;

  /*! Aproximate line where this type is declared */
  virtual uint32_t& line() = 0;

  /*! File ID associated with the instance */
  virtual unsigned int& inst_file_id() = 0;

  /*! Aproximate line where this instance is instantiated */
  virtual uint32_t& inst_line() = 0;

  /*! Instance ID for the instance */
  virtual unsigned int& instance_id() = 0;

  /*! Pointer to parent instance */
  scp_base* parent_scp = nullptr;

  /*! Map from instance name to child scope instance */
  std::unordered_map<std::string,scp_base*> child_scp_insts;

  /*! Map from instance name to covergroup instance */
  std::unordered_map<std::string,cvg_base*> cvg_insts;

  /*! Destructor */
  virtual ~scp_base() { }

};

/*!
 *  \class cvg_metadata fc_base.hpp
 *  \brief Representation for covergroup type
 *
 *  Covergroup type data which holds the type name and type specific data of a covergroup
 */
struct cvg_metadata
{

  /*! Covergroup type options */
  cvg_type_option type_option;

  /*! Covergroup type name */
  std::string type_name;

  /*! Scope type name this covergroup is defined within */
  std::string scp_type_name;

  /*! File id where this covergroup is defined */
  unsigned int file_id;

  /*! Line in file where this covergroup is defined */
  uint32_t line;

  /*! Vector of covergroup instances of this covergroup type */
  std::vector<cvg_base_data_model*> cvg_insts;

  /*! Destructor */
  virtual ~cvg_metadata()
  {
    for(auto cvg_it : cvg_insts)
    {
      delete cvg_it;
    }
  }

};

/*!
 *  \class scp_metadata fc_base.hpp
 *  \brief Representation for scope type
 *
 *  Scope type data which holds the type name and type specific data of a scope
 */
struct scp_metadata 
{

  /*! Name of the scope type */
  std::string type_name;
 
  /*! File ID associated with where this type is declared */
  unsigned int file_id;

  /*! Aproximate line where this type is declared */
  uint32_t line;

  /*! Map from covergroup type name to covergroup type */
  std::unordered_map<std::string,cvg_metadata*> cvg_type_table;

  /*! Vector of scope instances of this scope type */
  std::vector<scp_base_data_model*> scp_insts;

  /*! Destructor */
  virtual ~scp_metadata()
  {
    for(auto cvg_it : cvg_type_table)
    {
      delete cvg_it.second;
    }
    for(auto scp_it : scp_insts)
    {
      delete scp_it;
    }
  }

};



class illegal_bin_sample_exception : public std::exception {
private:
  std::string strconcat(const std::string& last) const { return last; }
  template <typename... Args>
  std::string strconcat(const std::string& head, Args... args) const {
    return head + strconcat(args...);
  }

  std::string value_hit;
  std::string bin_name;
  std::string cvp_name;
  std::string cvg_name;
  std::string message;

  void update_message() {
    message = strconcat("Illegal sample in [", cvg_name, "/", cvp_name, "/", bin_name, "] on value [", value_hit, "]!");
  }
public:
  void update_bin_info(const std::string& bin_name, const std::string& value) {
    this->bin_name = bin_name;
    this->value_hit = value;
    update_message();
  }
  void update_cvp_info(const std::string& cvp_name) {
    this->cvp_name = cvp_name;
    update_message();
  }
  void update_cvg_info(const std::string& cvg_name) {
    this->cvg_name = cvg_name;
    update_message();
  }

  const char * what () const throw () {
    return message.c_str();
  }
};

} // namespace fc4sc

#endif /* FC4SC_BASE_HPP */
