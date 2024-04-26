#pragma once
#include <string>
#include <xdp/libxdp.h>

class XdpProgram {
private:
  xdp_program* _program;
  int _if_index = 0;
  xdp_attach_mode _mode = XDP_MODE_UNSPEC;

  XdpProgram(xdp_program* pointer, int if_index); // use from_bpf_obj

public:
  struct BpfFileDescriptor {
    int descriptor;
  };

  XdpProgram(const XdpProgram&) = delete;
  XdpProgram(XdpProgram&&) = delete;
  XdpProgram& operator=(const XdpProgram&) = delete;
  XdpProgram& operator=(XdpProgram&&) = delete;
  ~XdpProgram();

  void attach();
  bpf_object* bpf_object_ptr();
  void update_map(const std::string& map_name, void* value);
  BpfFileDescriptor find_map_fd(const std::string& map_name);

  static XdpProgram from_bpf_obj(bpf_object* objects, int if_index);
};
