#include "main.skel.h"

class BpfProgram {
private:
  main_bpf* _program;

  /**
   * Use the factory method (create) instead.
   */
  explicit BpfProgram(main_bpf* program);

public:
  BpfProgram(const BpfProgram&) = delete;
  BpfProgram(BpfProgram&&) = delete;
  BpfProgram& operator=(const BpfProgram&) = delete;
  BpfProgram& operator=(BpfProgram&&) = delete;

  /**
   * Destroyes a BPF program.
   */
  ~BpfProgram();

  /**
   * Getter for a BPF program.
   * @return A BPF handle.
   */
  main_bpf* get();

  /**
   * A factory method for creating a BPF program.
   * @return A wrapper around a BPF program.
   */
  static BpfProgram create();
};
