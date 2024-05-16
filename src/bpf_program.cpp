#include "bpf_program.hxx"
#include "utils.hxx"

BpfProgram::BpfProgram(main_bpf* program)
  : _program(program){};

BpfProgram::~BpfProgram()
{
  main_bpf::destroy(_program);
  print("The eBPF program was released");
}

main_bpf*
BpfProgram::get()
{
  return _program;
}

BpfProgram
BpfProgram::create()
{
  auto program = main_bpf::open();
  if (!program) {
    throw std::runtime_error("Can't open eBPF program");
  }
  return BpfProgram(program);
}
