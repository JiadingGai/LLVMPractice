#ifndef __KERNEL_INFO_READER_H__
#define __KERNEL_INFO_READER_H_

#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <sstream>
#include <string>
#include <stdio.h>

using namespace llvm;

class CL_KernelInfo {

private:
  enum SPIR_ADDR_QUAL {
    SPIR_ADDR_PRIVATE,
    SPIR_ADDR_GLOBAL,
    SPIR_ADDR_CONSTANT,
    SPIR_ADDR_LOCAL
  };

  /* SPIR specs: */
  enum SPIR_ACCE_QUAL {
    SPIR_ACCESS_READ_ONLY,
    SPIR_ACCESS_WRITE_ONLY,
    SPIR_ACCESS_READ_WRITE,
    SPIR_ACCESS_NONE
  };

  static const char* SOCL_ADDR_QUAL[4];
  static const char* SOCL_ACCE_QUAL[4];

  unsigned int _cl_kernel_num;
  std::vector<std::string> kernel_names;
  std::vector<unsigned int> kernel_num_args;
  std::vector<std::string> attr_names;
  std::vector<std::vector<unsigned int> > wkgp_size_hints;
  std::vector<std::vector<unsigned int> > reqd_wkgp_sizes;
  std::vector<unsigned long long> local_mem_sizes;
  std::vector<unsigned long long> priv_mem_sizes;
  std::vector<std::string> addr_qualifier;
  std::vector<std::string> access_qualifier;
  std::vector<std::string> arg_type_names;
  std::vector<unsigned int> arg_type_qual;
  std::vector<std::string> arg_names;
  std::vector<unsigned int> kernel_dims;

  std::vector<std::vector<Type*> > kernel_arg_types;
  std::vector<std::vector<std::string> > arg_names_2D;

public:
  CL_KernelInfo(Module &M);
  void CL_KernelInfo_DEBUG();

  unsigned int get_cl_kernel_num();
  const std::vector<std::string>& get_kernel_names();
  const std::vector<unsigned int>& get_kernel_num_args();
  const std::vector<std::string>& get_attr_names();
  const std::vector<std::vector<unsigned int> >& get_wkgp_size_hints();
  const std::vector<std::vector<unsigned int> >& get_reqd_wkgp_sizes();
  const std::vector<unsigned long long>& get_local_mem_sizes();
  const std::vector<unsigned long long>& get_priv_mem_sizes();
  const std::vector<std::string>& get_addr_qualifier();
  const std::vector<std::string>& get_access_qualifier();
  const std::vector<std::string>& get_arg_type_names();
  const std::vector<unsigned int>& get_arg_type_qual();
  const std::vector<std::string>& get_arg_names();
  const std::vector<unsigned int>& get_kernel_dims();

  const std::vector<std::vector<Type*> >& get_kernel_arg_types();
  const std::vector<std::vector<std::string> >& get_arg_names_2D();

};

#endif // __KERNEL_INFO_READER_H__
