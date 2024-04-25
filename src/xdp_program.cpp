#include <format>
#include <xdp/libxdp.h>

#include "xdp_program.hxx"
#include "utils.hxx"

XdpProgram::XdpProgram(xdp_program* pointer, int if_index)
  : _pointer(pointer)
  , _if_index(if_index)
{}

XdpProgram::~XdpProgram() {
  xdp_program__detach(_pointer, _if_index, _mode, 0);
  xdp_program__close(_pointer);
  print("The XDP program was released");
}

void XdpProgram::attach() {
  auto error = xdp_program__attach(_pointer, _if_index, _mode, 0);
  if (error) {
    throw std::runtime_error(std::format("Can't attach XDP program to interface. Err code: {}", error));
  }
}

bpf_object* XdpProgram::bpf_object_ptr() {
  return xdp_program__bpf_obj(_pointer);
}

XdpProgram XdpProgram::from_bpf_obj(bpf_object* objects, int if_index) {
  auto handle = xdp_program__from_bpf_obj(objects, "xdp");
  auto error = libxdp_get_error(handle);
  if (error) {
    throw std::runtime_error("Can't create XDP program");
  }
  return {handle, if_index};
}
