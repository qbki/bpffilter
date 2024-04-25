#pragma once
#include <xdp/libxdp.h>

class XdpProgram {
private:
  xdp_program* _pointer;
  int _if_index = 0;
  xdp_attach_mode _mode = XDP_MODE_UNSPEC;

public:
  XdpProgram(xdp_program* pointer, int if_index); // use from_bpf_obj
  XdpProgram(const XdpProgram&) = delete;
  XdpProgram(XdpProgram&&) = delete;
  XdpProgram& operator=(const XdpProgram&) = delete;
  XdpProgram& operator=(XdpProgram&&) = delete;
  ~XdpProgram();

  void attach();
  bpf_object* bpf_object_ptr();

  static XdpProgram from_bpf_obj(bpf_object* objects, int if_index);
};
