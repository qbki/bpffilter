#include "main.skel.h"

class BpfProgram {
private:
  main_bpf* _program;

  explicit BpfProgram(main_bpf* program); // use create

public:
  BpfProgram(const BpfProgram&) = delete;
  BpfProgram(BpfProgram&&) = delete;
  BpfProgram& operator=(const BpfProgram&) = delete;
  BpfProgram& operator=(BpfProgram&&) = delete;
  ~BpfProgram();

  main_bpf* get();

  static BpfProgram create();
};
