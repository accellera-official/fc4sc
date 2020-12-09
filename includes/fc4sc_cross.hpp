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
 \file cross.hpp
 \brief Helper functions and interfaces

 This file contains helper functions and the base classes
 for bins, coverpoints and covergroups.
 */

#ifndef FC4SC_CROSS_HPP
#define FC4SC_CROSS_HPP

#include <tuple>
#include "fc4sc_base.hpp"
#include "fc4sc_bin.hpp"
#include "fc4sc_coverpoint.hpp"
#include "fc4sc_binsof.hpp"

namespace fc4sc
{

/*!
 *  \class crs_data_model fc_base.hpp
 *  \brief class for storing coverage data of crosses
 *
 *  Data storage for crosses
 */
class cross_data_model : public cross_base_data_model {
public:

  /*! Hit cross bins storage */
  std::map<std::vector<size_t>, uint64_t> bins;

  /*! Get cross bins storage */
  virtual const std::map<std::vector<size_t>, uint64_t>& get_cross_bins() const
  {
    return bins;
  }

  uint64_t size() const
  {
    uint64_t total = 1;
    for (auto& cvp : cross_cvps) 
    {
      total *= cvp->size();
    }
    return total;
  }

  void accept_visitor(covVisitorBase& visitor)
  {
    visitor.visit(*this);
  }

};

/*!
 * \brief Defines a class for crosses
 * \tparam Args Type of coverpoints being crossed
 */
template <typename... Args>
class cross : public cross_base
{

  friend class dynamic_covergroup_factory;

  /*! Pointer to this object's data */
  cross_data_model* crs_data = new cross_data_model;

  /*! For ensuring the object's data has not been deallocated */
  std::weak_ptr<bool> valid_data = crs_data->valid;

  /*! Sampling switch */
  bool collect = true;

  /*! Total number of bins in this cross */
  uint64_t total_coverpoints = 0;

  /*!
   *  \brief Create a dynamic copy of this cross with same type definition
   */
  cvp_base* create_instance(cvg_base* cvg_inst)
  {
    cross<Args...>* crs = new cross<Args...>;
    crs->total_coverpoints = this->total_coverpoints;
    crs->crs_data->name = this->crs_data->name;
    for (auto cvp : this->cvps_vec)
    {
      crs->cvps_vec.push_back(&(cvg_inst->get_coverpoint(cvp->name())));
      crs->crs_data->cross_cvps.push_back(cvg_inst->get_coverpoint(cvp->name()).get_data());
    }
    return crs;
  }

  /*!
   * \brief Helper function to recursively determine number of bins in a cross
   * \tparam Head Type of current processed coverpoint
   * \tparam Tail Rest of coverpoints
   * \param h Current coverpoint
   * \param t Rest of coverpoints
   * \returns number of bins in this cross
   */
  template <typename Head, typename... Tail>
  int get_size(Head h, Tail... t)
  {
    return h->size() * get_size(t...);
  }

  /*!
   * \brief End of recursion
   * \returns Default value 1
   */
  int get_size()
  {
    return 1;
  }

  /*!
   *  \brief Helper function to check if a sampled value is in a cross
   *  \tparam k Index of the currently checked element
   *  \tparam Head Type of current processed value
   *  \tparam Tail Rest of sampled values
   *  \param found Storage of hit bins for each coverpoint
   *  \param h Current sample value checked
   *  \param t Rest of sample values
   *  \returns True if each value is in its corresponding coverpoint
   */
  template <size_t k, typename Head, typename... Tail>
  bool check(std::vector<std::vector<size_t>> &found, Head h, Tail... t)
  {

    // See if the current value is in its coverpoint, and where
    std::vector<size_t> bin_indexes = (static_cast<coverpoint<Head> *>(cvps_vec[k - 1]))->get_bin_index(h);

    // If atleast one bin has it
    bool found_in_cvp = bin_indexes.size() > 0;

    if (!found_in_cvp)
    {
      return false;
    }

    // Store bins for this coverpoint
    found.push_back(bin_indexes);

    // Recurse
    return (k == 1) ? (found_in_cvp) : (check<k - 1>(found, t...) && (found_in_cvp));
  }

  /*!
   * \brief End of recursion
   */
  template <size_t k>
  bool check(std::vector<std::vector<size_t>> &found) {
    (void)found; // safe warning suppression
    return true;
  }

  /*!
   *  \brief Private constructor used by dynamic_covergroups
   *  \param cross name
   *  \param args Coverpoints to be crossed
   */
  template <typename... Restrictions>
  cross(const std::string& name, coverpoint<Args> *... args, Restrictions... binsofs) : cross(binsofs...) 
  {
    this->crs_data->name = name;

    total_coverpoints = get_size(args...);
    cvps_vec = std::vector<cvp_base*>{args...};

    std::reverse(cvps_vec.begin(), cvps_vec.end());
    for(auto cvp_it : cvps_vec) {
      crs_data->cross_cvps.push_back(cvp_it->get_data());
    }
  };

public:

  /*! Crossed coverpoints storage */
  std::vector<cvp_base *> cvps_vec;

  /*!
   *  \brief Main constructor
   *  \param n Parent covergroup
   *  \param args Coverpoints to be crossed
   */
  template <typename... Restrictions>
  cross(cvg_base *n, coverpoint<Args> *... args, Restrictions... binsofs) : cross(binsofs...) 
  {
    n->cvps.push_back(this);
    n->get_cvg_data()->add_cvp_data(crs_data);

    total_coverpoints = get_size(args...);
    cvps_vec = std::vector<cvp_base*>{args...};

    std::reverse(cvps_vec.begin(), cvps_vec.end());
    for(auto cvp_it : cvps_vec) {
      crs_data->cross_cvps.push_back(cvp_it->get_data());
    }
  };

  
  template <typename... Restrictions, typename Select>
  cross(binsof<Select> binsof_inst,  Restrictions... binsofs) : cross (binsofs...) {
    std::cerr << "consume binsof\n";
    static_cast<void>(binsof_inst);
  }

  cross(cvg_base *n, const std::string& name, coverpoint<Args> *... args) : cross(n, args...) {
    this->crs_data->name = name;
  };


  /*!
   *  \brief Default constructor
   */
  cross(){}

  /*!
   *  \brief Sampling function at cross level
   */
  virtual void sample() 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    if (!this->collect) return;
    std::vector <size_t> hit_bins;
    for (auto& cvp : cvps_vec) {
      if (cvp->last_sample_success) {
        hit_bins.push_back(cvp->last_bin_index_hit);
      }
      else {
        crs_data->misses++;
        return;
      }
    }
    crs_data->bins[hit_bins]++;
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

    int covered = 0;
    //int total = total_coverpoints;
    int total = this->size();

    if (total == 0)
      return (this->crs_data->option.weight == 0) ? 100 : 0;

    for (auto it : crs_data->bins)
    {
      if (it.second >= crs_data->option.at_least)
        covered++;
    }

    double real = 100.0 * covered / total;
    return (real >= this->crs_data->option.goal) ? 100 : real;
  }

  /*!
   *  \brief Computes coverage for this instance
   *  \param covered Number of covered bins
   *  \param total Total number of bins in this cross
   *  \returns Coverage value as a double between 0 and 100
   */
  double get_inst_coverage(int &covered, int &total) const 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    //total = total_coverpoints;
    total = this->size();
    covered = 0;

    for (auto it : crs_data->bins)
    {
      if (it.second >= crs_data->option.at_least)
        covered++;
    }

    if (total == 0)
      return (this->crs_data->option.weight == 0) ? 100 : 0;

    return covered / total;
  }

  cvp_base_data_model* get_data()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return crs_data;
  }

  /*!
   * \brief Enables sampling
   */
  void start()
  {
    collect = true;
  };

  /*!
   * \brief Disables sampling
   */
  void stop()
  {
    collect = false;
  };


  /*!
   * \brief Returns instance name
   */
  std::string& name() 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return crs_data->name;
  }

  /*!
   * \brief Returns instance options
   */
  cross_option& option() 
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return crs_data->option;
  }

  /*!
   * \brief Returns cross bin misses
   */
  uint64_t get_misses()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return crs_data->misses;
  }

  /*!
   * \brief Returns cross coverage weight
   */
  uint64_t get_weight()
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return this->crs_data->option.weight;
  };

  uint64_t size() const
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    uint64_t total = 1;
    for (auto& cvp : cvps_vec) 
    {
      total *= cvp->size();
    }
    return total;
    //return total_coverpoints;
  };

  /*! Get cross bins storage */
  const std::map<std::vector<size_t>, uint64_t>& get_cross_bins() const
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return crs_data->bins;
  }

  /*! Get crossed coverpoints storage */
  const std::vector<cvp_base *>& get_cross_coverpoints() const
  {
    if(valid_data.use_count() == 0) {
      std::cerr << "Error: coverage data has been deleted\n";
      throw("Error: coverage data has been deleted");
    }

    return cvps_vec;
  }

};

} // namespace fc4sc

#endif /* FC4SC_CROSS_HPP */
