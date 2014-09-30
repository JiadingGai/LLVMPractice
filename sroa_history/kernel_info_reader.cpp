#include "kernel_info_reader.h"

using namespace llvm;

const char* CL_KernelInfo::SOCL_ADDR_QUAL[4] = {"0", "3", "2", "1" };
const char* CL_KernelInfo::SOCL_ACCE_QUAL[4] = {"3", "2", "1", "0"};

// The constructor will gather all the information in one go
CL_KernelInfo::CL_KernelInfo(Module &M) {

  // @ _cl_kernel_num
  NamedMDNode * oclKernelmd = M.getNamedMetadata("opencl.kernels");
  _cl_kernel_num = oclKernelmd->getNumOperands();

  // Loop through all the kernels and gather information
  for(unsigned int nk = 0; nk < _cl_kernel_num; ++nk) 
  {
    MDNode * kernel_Nd = dyn_cast<MDNode>(oclKernelmd->getOperand(nk));
    // First operand is always the kernel call
    Value * kernelVal = kernel_Nd->getOperand(0);
    // Get number of metadata this kernel has
    unsigned int num_md = kernel_Nd->getNumOperands();

    // Save actual types
    Function * kernelFunc = dyn_cast<Function>(kernelVal);
    std::vector<Type* > arg_types;
    for(Function::arg_iterator ai = kernelFunc->arg_begin(); ai != kernelFunc->arg_end(); ++ai)
    {
      Argument &arg = *ai;
      if(arg.getType()->isPointerTy())
      {
        // remove address space qual from pointer
        arg_types.push_back(PointerType::getUnqual(arg.getType()->getPointerElementType()));
      } else
      {
        arg_types.push_back(arg.getType());
      }
      
    }
    kernel_arg_types.push_back(arg_types);

    // @ _cl_kernel_names[]
    StringRef name = kernelVal->getName();
    kernel_names.push_back(name.str());

    // in preparation for kernel attributes
    std::vector<unsigned int> zeroVal(3);
    wkgp_size_hints.push_back(zeroVal);
    reqd_wkgp_sizes.push_back(zeroVal);

    // if no special attributes
    attr_names.push_back("");

    // read additional metadata
    for(unsigned int md = 1; md < num_md; ++md) 
    {

      // get name of metadata
      MDNode * info_Nd = dyn_cast<MDNode>(kernel_Nd->getOperand(md));
      std::string infoName = info_Nd->getOperand(0)->getName().str();
      if(infoName.compare("kernel_arg_addr_space") == 0) 
      {
        // @ _cl_kernel_num_args[]
        kernel_num_args.push_back(info_Nd->getNumOperands() - 1);

        // _cl_kernel_arg_address_qualifier[]
        std::string addr_qual_prefix("");

        for(unsigned int i = 1; i < kernel_num_args.at(nk) + 1; ++i) {
          ConstantInt * constint = dyn_cast<ConstantInt>(info_Nd->getOperand(i));
          addr_qual_prefix += SOCL_ADDR_QUAL[constint->getZExtValue()];
        }

        addr_qualifier.push_back(addr_qual_prefix);
      }
      else if(infoName.compare("kernel_arg_access_qual") == 0) 
      {
        // _cl_kernel_arg_access_qualifier[]
        std::string acce_qual_prefix("");
        for(unsigned int i = 1; i < kernel_num_args.at(nk) + 1; ++i) {

          Value * acce_qual = info_Nd->getOperand(i);
          std::string acce_qual_name = acce_qual->getName().str();

          if(acce_qual_name.compare("read_only") == 0)
          {
            acce_qual_prefix += "3";
          }
          else if(acce_qual_name.compare("write_only") == 0)
          {
            acce_qual_prefix += "2";
          }
          else if(acce_qual_name.compare("read_write") == 0)
          {
            acce_qual_prefix += "1";
          }
          else  // none 
          {
            acce_qual_prefix += "0";
          }
        }

        access_qualifier.push_back(acce_qual_prefix);

      }
      else if(infoName.compare("kernel_arg_type") == 0) 
      {
        // _cl_kernel_arg_type_name[]

        for(unsigned int i = 1; i < kernel_num_args.at(nk) + 1; ++i) {
          Value * arg_type_name_val = info_Nd->getOperand(i);
          std::string arg_type_name = arg_type_name_val->getName().str();

          arg_type_names.push_back(arg_type_name);
        }

      }
      else if(infoName.compare("kernel_arg_type_qual") == 0) 
      {
        // _cl_kernel_arg_type_qualifier[]

        for(unsigned int i = 1; i < kernel_num_args.at(nk) + 1; ++i) {
          Value * type_qual_val = info_Nd->getOperand(i);
          std::string type_qual = type_qual_val->getName().str();

          unsigned int temp_val = 0;

          if(type_qual.find("const") != std::string::npos)
          {
            temp_val += 1;
          }
          if(type_qual.find("restrict") != std::string::npos)
          {
            temp_val += 2;
          }
          if(type_qual.find("volatile") != std::string::npos)
          {
            temp_val += 4;
          }

          arg_type_qual.push_back(temp_val);

        }

      }
      else if(infoName.compare("kernel_arg_name") == 0)
      {

        // _cl_kernel_arg_name[]
        for(unsigned int i = 1; i < kernel_num_args.at(nk) + 1; ++i) {
          Value * arg_type_name_val = info_Nd->getOperand(i);
          std::string arg_type_name = arg_type_name_val->getName().str();

          arg_names.push_back(arg_type_name);

        }

      }
      else {

        // _cl_kernel_attributes[]

        if(infoName.compare("vec_type_hint") == 0)
        {
          // clang is giving me errors for float4 and such
          // this is incomplete
          std::string prefix = "__attribute__((vec_type_hint(";
          std::string suffix = ")))";
          std::stringstream ss;
          ss << "vectype";
          prefix += ss.str() + suffix;

          attr_names.back() = prefix;
        } 
        else 
        {
          ConstantInt * constfirst  = dyn_cast<ConstantInt>(info_Nd->getOperand(1));
          ConstantInt * constsecond = dyn_cast<ConstantInt>(info_Nd->getOperand(2));
          ConstantInt * constthird  = dyn_cast<ConstantInt>(info_Nd->getOperand(3));
          
          // sext or zext?
          unsigned int first = (unsigned int)constfirst->getZExtValue();
          unsigned int second = (unsigned int)constsecond->getZExtValue();
          unsigned int third = (unsigned int)constthird->getZExtValue();

          std::vector<unsigned int> workgroups(3);
          workgroups.at(0) = first;
          workgroups.at(1) = second;
          workgroups.at(2) = third;

          // _cl_kernel_work_group_size_hint[][3]
          if(infoName.compare("work_group_size_hint") == 0)
          {

            wkgp_size_hints.back() = workgroups;

            std::string prefix = "__attribute__((work_group_size_hint(";
            std::string suffix = ")))";
            std::stringstream ss;
            ss << first << "," << second << "," << third;
            prefix += ss.str() + suffix;

            attr_names.back() = prefix;
          }
          // _cl_kernel_reqd_work_group_size[][3]
          else if(infoName.compare("reqd_work_group_size") == 0)
          {

            reqd_wkgp_sizes.back() = workgroups;

            std::string prefix = "__attribute__((reqd_work_group_size(";
            std::string suffix = ")))";
            std::stringstream ss;
            ss << first << "," << second << "," << third;
            prefix += ss.str() + suffix;

            attr_names.back() = prefix;
          }
          else
          {
            DEBUG(errs() << "Invalid attribute: " << infoName);
          }
        } // vec_type_hint
      } // else metadata names
    } // metadata for loop

    // _cl_kernel_local_mem_size[]
    // This looks for __local qualifiers of declaration statements. 
    // If it is a pointer, it will return 0, otherwise it will return
    // size * numDecl in the kernel. However, clang does not accept
    // __local without a pointer. 
    // Look inside TransformLocalVD.cpp
    unsigned long long local_size = 0;

    local_mem_sizes.push_back(local_size);

    // _cl_kernel_local_mem_size[]
    // sum of the number of bytes of all variable declarations
    // with -O0, clang will generate variable declarations as alloca without
    // .addr appended to the v.register label. In addition, the alignment
    // will give me the size of type without having to maintain a list
    unsigned long long priv_size = 0;
    
    Function * f = dyn_cast<Function>(kernelVal);
    for(inst_iterator I = inst_begin(f); I != inst_end(f); ++I) {
      Instruction & inst = *I;
      if(AllocaInst *AI = dyn_cast<AllocaInst>(&inst))
        if(! AI->getName().endswith(".addr"))
          priv_size += (unsigned long long)AI->getAlignment();
    }
    
    priv_mem_sizes.push_back(priv_size);

      // _cl_kernel_dim[]
    kernel_dims.push_back(3);

  } // kernel for loop

}

unsigned int CL_KernelInfo::get_cl_kernel_num() 
{
  return _cl_kernel_num;
}

const std::vector<std::string>& CL_KernelInfo::get_kernel_names()
{
  return kernel_names;
}

const std::vector<unsigned int>& CL_KernelInfo::get_kernel_num_args()
{
  return kernel_num_args;
}

const std::vector<std::string>& CL_KernelInfo::get_attr_names()
{
  return attr_names;
}

const std::vector<std::vector<unsigned int> >& CL_KernelInfo::get_wkgp_size_hints()
{
  return wkgp_size_hints;
}

const std::vector<std::vector<unsigned int> >& CL_KernelInfo::get_reqd_wkgp_sizes()
{
  return reqd_wkgp_sizes;
}

const std::vector<unsigned long long>& CL_KernelInfo::get_local_mem_sizes()
{
  return local_mem_sizes;
}

const std::vector<unsigned long long>& CL_KernelInfo::get_priv_mem_sizes()
{
  return priv_mem_sizes;
}

const std::vector<std::string>& CL_KernelInfo::get_addr_qualifier()
{
  return addr_qualifier;
}

const std::vector<std::string>& CL_KernelInfo::get_access_qualifier()
{
  return access_qualifier;
}

const std::vector<std::string>& CL_KernelInfo::get_arg_type_names()
{
  return arg_type_names;
}

const std::vector<unsigned int>& CL_KernelInfo::get_arg_type_qual()
{
  return arg_type_qual;
}

const std::vector<std::string>& CL_KernelInfo::get_arg_names()
{
  return arg_names;
}

const std::vector<unsigned int>& CL_KernelInfo::get_kernel_dims()
{
  return kernel_dims;
}

const std::vector<std::vector<Type*> >& CL_KernelInfo::get_kernel_arg_types()
{
  return kernel_arg_types;
}

const std::vector<std::vector<std::string> >& CL_KernelInfo::get_arg_names_2D()
{
  unsigned int str_id = 0;

  for(unsigned int k = 0; k < _cl_kernel_num; ++k)
  {
    std::vector<std::string> one_kernel;
    unsigned int num_args = kernel_num_args.at(k);
    for(unsigned int ai = 0; ai < num_args; ++ai, ++str_id)
    {
      one_kernel.push_back(arg_names.at(str_id));
    }
    arg_names_2D.push_back(one_kernel);
  }

  return arg_names_2D;
}
