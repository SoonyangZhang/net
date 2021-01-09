#pragma once
#include <memory>
#include <utility>
namespace basic{
#define BASE_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;          \
  TypeName& operator=(const TypeName&) = delete    
  
#define UNUSED(expr) do { (void)(expr); } while (0)
}