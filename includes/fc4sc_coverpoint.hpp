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
 \file coverpoint.hpp
 \brief Helper functions and interfaces

 This file contains helper functions and the base classes
 for bins, coverpoints and covergroups.
 */

#ifndef FC4SC_COVERPOINT_HPP
#define FC4SC_COVERPOINT_HPP

#include <type_traits>
#include <list>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <algorithm> // std::find

#include "fc4sc_bin.hpp"

namespace fc4sc
{

// Friend classes forward declaration
template <typename T>
class binsof;

template <typename ... Args>
class cross;

/*!
 * \brief Defines a class for coverpoints
 * \tparam T Type of bins that this coverpoint is holding
 */
template <class T>
class coverpoint;

class covergroup;

template<typename T, typename U>
class dynamic_coverpoint_factory;
class dynamic_covergroup_factory;

/*!
 *  \class coverpoint_data_model fc_coverpoint.hpp
 *  \brief class for storing coverage data of coverpoints
 *
 *  Data storage for coverpoints
 */
class coverpoint_data_model : public coverpoint_base_data_model {
public:

  /*! String-ified sample expression. */
  std::string sample_expression_str;

  /*!
   * String-ified sample condition. It can be used for reporting.
   * When the string is empty, no sample condition is registered.
   */
  std::string sample_condition_str;

  uint64_t size() const {
    return bins_data.size();
  }

  std::string& get_sample_expression_str()
  {
    return sample_expression_str;
  }

  std::string& get_sample_condition_str()
  {
    return sample_condition_str;
  }

  void accept_visitor(covVisitorBase& visitor)
  {
    visitor.visit(*this);
  }

};

template <class T>
class coverpoint final : public coverpoint_base
{
private:
  // static check to make sure that the coverpoint is templated by an integral type
  static_assert(std::is_integral<T>::value, "Type must be integral!");
  /*
   * The covergroup class needs access to private members of the coverpoint
   * in order to set the sample condition, expression and name.
   */
  friend class covergroup;
  // friend functions that need access to the bin vectors -> add_to_cvp(coverpoint<T>&)
  friend class bin<T>;
  friend class bin_array<T>;
  friend class ignore_bin<T>;
  friend class illegal_bin<T>;

  template<typename U, typename V>
  friend class dynamic_coverpoint_factory;
  friend class dynamic_covergroup_factory;

  typedef std::pair<unsigned int, unsigned int> bin_range_t;

  struct IntervalComp {
    bool operator() (fc4sc::interval_t<T> a, fc4sc::interval_t<T> b) {
      return a.second < b.second;
    }
  };

  typedef std::map<fc4sc::interval_t<T>,std::vector<bin_range_t>,IntervalComp> interval_map_t;
  typedef typename std::map<fc4sc::interval_t<T>,std::vector<bin_range_t>,IntervalComp>::iterator interval_map_iterator_t;

  bool has_sample_expression = false;

  /*! Pointer to this object's data */
  coverpoint_data_model* cvp_data = new coverpoint_data_model;

  /*! For ensuring the object's data has not been deallocated */
  std::weak_ptr<bool> valid_data = cvp_data->valid;

  /*! Expression used to sample the data */
  std::function<T()> sample_expression;

  /*!
   *  \brief Create a dynamic copy of this coverpoint with matching bin types
   */
  cvp_base* create_instance(cvg_base* cvg_inst)
  {
    coverpoint<T>* cvp = new coverpoint<T>;
    cvp->cvp_data->name = this->name();
    cvp->cvp_data->option = this->option();
    cvp->cvp_data->sample_expression_str = this->get_sample_expression_str();
    cvp->cvp_data->sample_condition_str = this->get_sample_condition_str();
    for(auto& bin_it : this->bins)
    {
      auto new_bin_data = new bin_data_model<T>;
      cvp->bins.push_back(bin_it);
      cvp->bins.back().bin_data = new_bin_data;
      cvp->cvp_data->bins_data.push_back(new_bin_data);
      *new_bin_data = *(bin_it.bin_data);
      std::fill(new_bin_data->interval_hits.begin(),new_bin_data->interval_hits.end(),0);
    }
    for(auto& bin_it : this->illegal_bins)
    {
      auto new_bin_data = new bin_data_model<T>;
      cvp->illegal_bins.push_back(bin_it);
      cvp->illegal_bins.back().bin_data = new_bin_data;
      *new_bin_data = *(bin_it.bin_data);
      cvp->cvp_data->illegal_bins_data.push_back(new_bin_data);
      std::fill(new_bin_data->interval_hits.begin(),new_bin_data->interval_hits.end(),0);
    }
    for(auto& bin_it : this->ignore_bins)
    {
      auto new_bin_data = new bin_data_model<T>;
      cvp->ignore_bins.push_back(bin_it);
      cvp->ignore_bins.back().bin_data = new_bin_data;
      *new_bin_data = *(bin_it.bin_data);
      cvp->cvp_data->ignore_bins_data.push_back(new_bin_data);
      std::fill(new_bin_data->interval_hits.begin(),new_bin_data->interval_hits.end(),0);
    }

    cvp->has_sample_expression = this->has_sample_expression;
    cvp->sample_expression = this->sample_expression;
    cvp->sample_condition = this->sample_condition;
    
    cvp->regular_interval_map = this->regular_interval_map;
    cvp->illegal_interval_map = this->illegal_interval_map;
    cvp->ignore_interval_map = this->ignore_interval_map;
    return cvp;
  }

  /*! Condition based on which the sampling takes place or not */
  std::function<bool()> sample_condition;

  // pointer sample variable (assigned via SAMPLE_POINT macro)
  T* sample_point = nullptr;

  /*! bins contained in this coverpoint */
  std::vector<bin<T>> bins;
  /*! Illegal bins contained in this coverpoint */
  std::vector<illegal_bin<T>> illegal_bins;
  /*! Ignore bins contained in this coverpoint */
  std::vector<ignore_bin<T>> ignore_bins;

  /*! Sampling switch */
  bool collect = true;

  coverpoint<T>& operator=(coverpoint<T>& rh) = delete;

  /*
   * kept for backwards compatibility with previous instantiation method:
   * coverpoint<int> cvp = coverpoint<int> (...);
   */
  // coverpoint(coverpoint<T>&& rh) = delete;

  /*!
   *  \brief Build interval map from bin
   */
  void build_interval_map(interval_map_t& interval_map, bin<T>& bin)
  {
    auto map_it = interval_map.begin();
    unsigned int i = 0;
    std::vector<bin_range_t> entry{ {0,i} };
    auto& elem_entry = entry[0];
    for(auto it = bin.bin_data->intervals.begin();it != bin.bin_data->intervals.end(); it++)
    {
      map_it = interval_map.insert(map_it,{*it, entry }); 
      elem_entry.second = ++i;
    }
  }


  /*!
   *  \brief Binary search in sorted map given a type T value
   */
  interval_map_iterator_t get_interval_bs(interval_map_t& interval_map, T val)
  {
    return interval_map.lower_bound(fc4sc::interval<T>(val,val));
  }


  /*!
   *  \brief Helper for slicing intervals
   */
  interval_map_iterator_t slice_helper(interval_map_t& interval_map,interval_map_iterator_t map_it, fc4sc::interval_t<T> i0, fc4sc::interval_t<T> i1)
  {
    auto bin_range_vec = map_it->second;
   //To slice, we must erase the interval
   //cannot change a key in-place in std::map until C++17
    map_it = interval_map.erase(map_it);
    map_it = interval_map.insert(map_it, {i1,bin_range_vec} );
    return interval_map.insert(map_it, {i0,bin_range_vec} );
  }


  /*!
   *  \brief Helper for slicing the intervals of overlapping bins
   */
  interval_map_iterator_t slice_intervals_map(interval_map_iterator_t hint_it, interval_map_t& interval_map, fc4sc::interval_t<T> range)
  {
    T start_cut = range.first;
    T end_cut = range.second;
    auto map_it = get_interval_bs(interval_map,end_cut);
    if(map_it != interval_map.end()) {
      auto it = map_it->first;
      if(end_cut > it.first && end_cut < it.second){
        map_it = slice_helper(interval_map,map_it,fc4sc::interval<T>(end_cut+1,it.second),fc4sc::interval<T>(it.first,end_cut));
      }
      else if(end_cut == it.first && it.first != it.second) {
        map_it = slice_helper(interval_map,map_it,fc4sc::interval<T>(it.first+1,it.second),fc4sc::interval<T>(end_cut,end_cut));
      }
    }
    map_it = get_interval_bs(interval_map,start_cut);
    if(map_it != interval_map.end()) {
      auto it = map_it->first;
      if(start_cut > it.first && start_cut < it.second ) {
        map_it = slice_helper(interval_map,map_it,fc4sc::interval<T>(it.first,start_cut-1),fc4sc::interval<T>(start_cut,it.second));
      }
      else if(start_cut == it.second && it.first != it.second) {
        map_it = slice_helper(interval_map,map_it,fc4sc::interval<T>(it.first,start_cut-1),fc4sc::interval<T>(start_cut,start_cut));
      }
    }
    return map_it;
  }

  /*!
   *  \brief Helper for adding new ranges to the flat map representation of the coverpoint
   */
  interval_map_iterator_t add_range_map(interval_map_iterator_t hint_it,interval_map_t& interval_map,bin_range_t bin_range_key,fc4sc::interval_t<T> range)
  {
    if(interval_map.empty() || range.second < hint_it->first.first) {
      std::vector<bin_range_t> bin_range_vec{bin_range_key};
      return std::next( interval_map.insert(hint_it,{range, bin_range_vec }) );
    }
    for(auto map_it = hint_it; map_it != interval_map.end(); map_it++)
    {
      auto it = map_it->first;
      if(range.first > range.second) {
        return map_it;
      }
      if(range.first <= it.first && range.second >= it.second) { 
	map_it->second.push_back(bin_range_key);
      }
      if(range.first < it.first) {
        std::vector<bin_range_t> bin_range_vec{bin_range_key};
        map_it = std::next(interval_map.insert(map_it, {fc4sc::interval<T>(range.first,it.first-1), bin_range_vec } ));
      }
      if(it.second + 1 > range.first)
        range.first = it.second + 1;
    }
    std::vector<bin_range_t> bin_range_vec{bin_range_key};
    interval_map.insert(interval_map.end(), {range, bin_range_vec } );
    return interval_map.end();
  }

  /*!
   *  \brief Helper for sorting the Flat map representation, adds a new bin to the coverpoint
   */
  void insert_intervals(interval_map_t& interval_map, bin<T>& new_bin, unsigned int bin_key)
  {
    if(interval_map.empty()) {
      build_interval_map(interval_map,new_bin);
      return;
    }
    auto hint_it = interval_map.begin();
    unsigned int i = 0;
    for(auto bin_it = new_bin.bin_data->intervals.begin(); bin_it != new_bin.bin_data->intervals.end(); bin_it++)
    {
      hint_it = slice_intervals_map(hint_it,interval_map,*bin_it);
      hint_it = add_range_map(hint_it,interval_map,{bin_key,i++},*bin_it);
    }
  }

  /*! Flat map representation of coverpoint's bin for binary search sampling */
  interval_map_t regular_interval_map;

  /*! Flat map representation of coverpoint's bin for binary search sampling */
  interval_map_t illegal_interval_map;

  /*! Flat map representation of coverpoint's bin for binary search sampling */
  interval_map_t ignore_interval_map;
 
  /*!
   *  \brief Sampling function at coverpoint level
   *  \param cvp_val Value to be sampled for this coverpoint
   *
   *  Takes a value and searches it in the owned bins
   */
  void sample(const T &cvp_val)  {
#ifdef FC4SC_DISABLE_SAMPLING
    return;
#endif
    if (!collect) return;
    this->last_sample_success = false;

    // 1) Search if the value is in the ignore bins
    auto range_it  = get_interval_bs(this->ignore_interval_map,cvp_val);
    if(range_it != this->ignore_interval_map.end()) {
      for(auto bin_range_it : range_it->second)
      {
        if (this->ignore_bins[bin_range_it.first].sample(cvp_val,bin_range_it.second)) {
          cvp_data->misses++;
          return;
        }
      }
    }

    // 2) Search if the value is in the illegal bins
    range_it  = get_interval_bs(this->illegal_interval_map,cvp_val);
    if(range_it != this->illegal_interval_map.end()) {
      for(auto bin_range_it : range_it->second)
      {
        try { this->illegal_bins[bin_range_it.first].sample(cvp_val,bin_range_it.second); }
        catch (illegal_bin_sample_exception &e) {
          e.update_cvp_info(this->cvp_data->name);
          throw e;
        }
      }
    }

    // 3) Sample regular bins    
    range_it  = get_interval_bs(this->regular_interval_map,cvp_val);
    if(range_it != this->regular_interval_map.end()) {
      for(auto bin_range_it : range_it->second)
      {
        if (this->bins[bin_range_it.first].sample(cvp_val,bin_range_it.second)) {
          this->last_bin_index_hit = bin_range_it.first;
          this->last_sample_success = true;
          if (this->stop_sample_on_first_bin_hit) return;
        }
      }
    }
    
    if (!this->last_sample_success) { cvp_data->misses++; }
  }

  /*! Default constructor */
  coverpoint() { }

  /*!
   *  \brief Constructor that registers a new default bin
   */
  template <typename... Args>
  coverpoint(bin<T> n, Args... args) : coverpoint(args...)
  {
    if (!n.is_empty())
    {
      std::reverse(n.bin_data->intervals.begin(), n.bin_data->intervals.end());
      //bins.push_back(n);
      n.add_to_cvp(*this);
    }
  }

  /*!
   *  \brief Constructor that a bin array
   */
  template < typename... Args>
  coverpoint(bin_array<T> n, Args... args) : coverpoint(args...)
  {
    n.add_to_cvp(*this);
  }

  /*!
   *  \brief Constructor that registers a new illegal bin
   */
  template <typename... Args>
  coverpoint(illegal_bin<T> n, Args... args) : coverpoint(args...)
  {
    if (!n.is_empty())
    {
      std::reverse(n.bin_data->intervals.begin(), n.bin_data->intervals.end());
      //illegal_bins.push_back(n);
      n.add_to_cvp(*this);
    }
  }

  /*!
   *  \brief Constructor that registers a new ignore bin
   */
  template <typename... Args>
  coverpoint(ignore_bin<T> n, Args... args) : coverpoint(args...)
  {
    if (!n.is_empty())
    {
      std::reverse(n.bin_data->intervals.begin(), n.bin_data->intervals.end());
      //ignore_bins.push_back(n);
      n.add_to_cvp(*this);
    }
  }

  /*!
   *  \brief Constructor that takes the parent covergroup.
   */
  template <typename... Args>
  coverpoint(const std::string& cvp_name, Args... args) : coverpoint(args...)
  {
    this->cvp_data->name = cvp_name;
  }

public:

  // Initialization constructor. Needed for the use of COVERGROUP macro;
  coverpoint(const coverpoint<T>& rh) = default;
  /*
   * Move assignment operator. Needed for the use of COVERGROUP macro. This should not be
   * "manually" used!
   */
  coverpoint<T>& operator=(coverpoint<T>&& rh) {
    /*
     * It is very important that the only fields moved from the right hand are
     * the bins. This is a needed assumption which makes the COVERPOINT macro
     * syntax work!
     */
    this->bins = std::move(rh.bins);
    this->ignore_bins = std::move(rh.ignore_bins);
    this->illegal_bins = std::move(rh.illegal_bins);
    this->regular_interval_map = std::move(rh.regular_interval_map);
    this->illegal_interval_map = std::move(rh.illegal_interval_map);
    this->ignore_interval_map = std::move(rh.ignore_interval_map);

    this->cvp_data->bins_data = rh.cvp_data->bins_data;
    this->cvp_data->illegal_bins_data = rh.cvp_data->illegal_bins_data;
    this->cvp_data->ignore_bins_data = rh.cvp_data->ignore_bins_data;
    rh.cvp_data->bins_data.clear();
    rh.cvp_data->illegal_bins_data.clear();
    rh.cvp_data->ignore_bins_data.clear();
    delete rh.cvp_data;
    return *this;
  }

  ~coverpoint() = default;

  /*!
   * \brief Initializer list constructor that receives a list of bin (of any types,
   * even mixed) and registers all the bins in this coverpoint.
   */
  coverpoint(std::initializer_list<const bin_wrapper<T>> list) {
    for (const bin_wrapper<T> &bin_w : list) {
      bin_w.get_bin()->add_to_cvp(*this);
    }
  }

  template <typename... Args>
  coverpoint(cvg_base *parent_cvg, Args... args) : coverpoint(args...) {
    static_assert(forbid_type<cvg_base *, Args...>::value, "Coverpoint constructor accepts only 1 parent covergroup pointer!");
    // Because the way that delegated constructors work, the coverpoint arguments
    // processed in the reverse order, resulting in a reversed vector of bins.
    //std::reverse(bins.begin(), bins.end());
    parent_cvg->register_cvp(this);

    // set strings here
    auto strings = parent_cvg->get_strings(this);
    this->sample_point = static_cast<T *>(std::get<0>(strings));
    this->cvp_data->name = std::get<1>(strings);
    this->cvp_data->sample_expression_str = std::get<2>(strings);
    parent_cvg->get_cvg_data()->add_cvp_data(this->cvp_data);
  }

  /*!
   *  Retrieves the number of regular bins
   */
  uint64_t size() const {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    return bins.size();
  }

  void sample() 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }
    if (has_sample_expression) {
      bool cond;
      try {
        cond = sample_condition();
      }  catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
	std::cerr << "sample_condition is not binded for coverpoint " << this->cvp_data->name << "\n";
	throw e;
      }
      if (cond) {
        try {
          this->sample(sample_expression());
	} catch(const std::exception& e) {
	  std::cerr << e.what() << "\n";
	  std::cerr << "sample_expression is not binded for coverpoint " << this->cvp_data->name << "\n";
	  throw e;
	}
      }
      else {
        // This is a fix so that crosses are not sampled if any of the coverpoints
        // used for crossing has a sample condition which is not met.
        this->last_sample_success = false;
        this->last_bin_index_hit = 0;
      }
    }
    else {
      this->sample(*sample_point);
    }
  }

  /*!
   *  \brief Computes coverage for this instance
   *  \returns Coverage value as a double between 0 and 100
   */
  double get_inst_coverage() const 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    if (bins.empty()) // no bins defined
      return (cvp_data->option.weight == 0) ? 100 : 0;

    double res = 0;
    for (auto &bin : bins)
      res += (bin.get_hitcount() >= cvp_data->option.at_least);

    double real = res * 100 / bins.size();

    return (real >= this->cvp_data->option.goal) ? 100 : real;
  }

  /*!
   *  \brief Computes coverage for this instance
   *  \param covered Number of covered bins
   *  \param total Total number of bins in this coverpoint
   *  \returns Coverage value as a double between 0 and 100
   */
  double get_inst_coverage(int &covered, int &total) const 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    double res = 0;
    covered = 0;
    total = bins.size();

    if (!bins.size())
    {
      total = 0;
      return (cvp_data->option.weight == 0) ? 100 : 0;
    }

    for (auto &bin : bins)
      res += (bin.get_hitcount() >= cvp_data->option.at_least);

    covered = res;
    double real = res * 100 / total;
    return (real >= this->cvp_data->option.goal) ? 100 : real;
  }

  /*!
   *  \brief Retrieves the number of hits of the bin with a particular index.
   *  The index order is the same as in the declaration of the coverpoint.
   *  This function will throw an error if the bin count is out of bounds!
   *  \returns the hit counter of a specified bin (by bin index).
   */
  uint64_t get_bin_hit_count(uint32_t bin_index) const
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    if (bin_index >= bins.size()) {// bin index out of bounds
      std::cerr << "FC4SC " << __FUNCTION__ << ": Error! bin_index argument "
          "is out of bounds. Passed value: [" << bin_index << "]" << std::endl
          << "Coverpoint [" << this->cvp_data->name << "] has [" << bins.size()
          << "] bins!" << std::endl;
      return 0;
    }

    return (this->bins[bin_index].get_hitcount());
  }

  /*!
   * \brief Enables sampling
   */
  void start()
  {
    collect = true;
  }

  /*!
   * \brief Enables sampling
   */
  void stop()
  {
    collect = false;
  }

  cvp_base_data_model* get_data()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return cvp_data;
  }

  /*!
   * \brief Returns instance name
   */
  std::string& name() 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return cvp_data->name;
  }

  /*!
   * \brief Returns instance options
   */
  cvp_option& option() 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return cvp_data->option;
  }

 /*!
   * \brief Returns coverpoint bin misses
   */
  uint64_t get_misses()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return cvp_data->misses;
  }

  /*!
   * \brief Returns coverpoint coverage weight
   */
  uint64_t get_weight()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return cvp_data->option.weight;
  }

  /*!
   * \brief Returns a vector of pointers to base_bin for introspection
   */
  std::vector<bin_base*> get_bins_base()
  {
    std::vector<bin_base*> bin_base_vec;
    for (auto &bin_it : bins)
    {
      bin_base_vec.push_back(&bin_it);
    }
    return bin_base_vec;
  }

  /*!
   * \brief Returns a vector of pointers to base_bin for introspection
   */
  std::vector<bin_base*> get_illegal_bins_base()
  {
    std::vector<bin_base*> bin_base_vec;
    for (auto &bin_it : illegal_bins)
    {
      bin_base_vec.push_back(&bin_it);
    }
    return bin_base_vec;
  }

  /*!
   * \brief Returns a vector of pointers to base_bin for introspection
   */
  std::vector<bin_base*> get_ignore_bins_base()
  {
    std::vector<bin_base*> bin_base_vec;
    for (auto &bin_it : ignore_bins)
    {
      bin_base_vec.push_back(&bin_it);
    }
    return bin_base_vec;
  }

  std::string& get_sample_expression_str()
  {
    return cvp_data->sample_expression_str;
  }

  std::string& get_sample_condition_str()
  {
    return cvp_data->sample_condition_str;
  }

};

/*!
 * \brief Defines a class to manage coverpoints in
 * dynamically defined covergroup
 * \tparam T Type of sample expression
 * \tparam U Type of sample condition expression
 */
template<typename T, typename U>
class dynamic_coverpoint_factory {

typedef typename std::function<T>::result_type ret_type;
typedef typename std::function<U>::result_type cond_ret_type;

friend class dynamic_covergroup_factory;

static_assert(std::is_same<bool,cond_ret_type>::value,"sample condition expression must return bool type");
  std::function<T> sample_expression;
  std::function<U> sample_condition;

  coverpoint<ret_type>* cvp_fact;

public:

  /*!
   * \brief binds variables to sample expression
   * \param Args must be same type as args for sample
   * expression
   */
  template<typename ... Args>
  void bind_sample(cvg_base& cvg, Args&... args)
  {
    static_assert(std::is_same<T,ret_type(Args...)>::value,"Argument variables must match coverpoint sample expression");
    cvp_base& cp_base = cvg.get_coverpoint(cvp_fact->cvp_data->name);
    coverpoint<ret_type>* cvp = dynamic_cast< coverpoint<ret_type>* >(&cp_base);
    if( cvp->sample_expression ) 
      throw ("Attempted to bind sample expression that is already binded in coverpoint " + cvp->cvp_data->name + " in covergroup " + cvg.name());
    cvp->sample_expression = std::bind(sample_expression,std::ref(args)...);
    if(!cvg.is_enabled()) std::cerr << "Warning: called bind_sample on disabled covergroup " << cvg.name() << "\n";
  }

  /*!
   * \brief binds variables to sample condition expression
   * \param Args must be same type as args for sample
   * condition expression
   */
  template<typename ... Args>
  void bind_condition(cvg_base& cvg, Args&... args)
  {
    static_assert(std::is_same<U,bool(Args...)>::value,"Argument variables must match coverpoint sample condition expression");
    cvp_base& cp_base = cvg.get_coverpoint(cvp_fact->cvp_data->name);
    coverpoint<ret_type>* cvp = dynamic_cast< coverpoint<ret_type>* >(&cp_base);
    if( cvp->sample_condition ) 
      throw ("Attempted to bind sample condition that is already binded in coverpoint " + cvp->name() + " in covergroup " + cvg.name());
    cvp->sample_condition = std::bind(sample_condition,std::ref(args)...);
    if(!cvg.is_enabled()) std::cerr << "Warning: called bind_condition on disabled covergroup " << cvg.name() << "\n";

  }

  /*!
   *  \brief Takes the bin name. Uses bin constructor
   *  \param bin_name Name of the bin
   *  \param args Rest of arguments
   */
  template<typename ... Args>
   void create_bin(const std::string &bin_name, Args... args)
  {
    bin<ret_type>(bin_name,args...).add_to_cvp(*cvp_fact);
  }

 /*!
   *  \brief Takes the bin name. Uses bin constructor
   *  \param bin_name Name of the bin
   *  \param args Rest of arguments
   */
  template<typename ... Args>
   void create_illegal_bin(const std::string &bin_name, Args... args)
  {
    illegal_bin<ret_type>(bin_name,args...).add_to_cvp(*cvp_fact);
  }

  /*!
   *  \brief Takes the bin name. Uses bin constructor
   *  \param bin_name Name of the bin
   *  \param args Rest of arguments
   */
  template<typename ... Args>
   void create_ignore_bin(const std::string &bin_name, Args... args)
  {
    ignore_bin<ret_type>(bin_name,args...).add_to_cvp(*cvp_fact);
  }

  /*!
   * \brief Constructs an bin_array which will split an interval into multiple
   * equal parts. The number of sub-intervals is specified via the count argument
   */
  void create_bin_array(const std::string &name, uint64_t count, interval_t<ret_type> interval)
  {
    bin_array<ret_type>(name,count,interval).add_to_cvp(*cvp_fact);
  }

  /*!
   * \brief Constructs an bin_array from a vector of intervals where each
   * interval will be nested by a bin
   */
  void  create_bin_array(const std::string &name, std::vector<interval_t<ret_type>>&& intvs)
  {
    bin_array<ret_type>(name,intvs).add_to_cvp(*cvp_fact);
  }

  /*!
   * \brief Constructs an bin_array from a vector of values where each
   * values will be nested by a bin
   */
  void create_bin_array(const std::string &name, const std::vector<ret_type>& intvs)
  {
    bin_array<ret_type>(name,intvs).add_to_cvp(*cvp_fact);
  }

};

} // namespace fc4sc

#endif /* FC4SC_COVERPOINT_HPP */
