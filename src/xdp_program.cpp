#include <format>
#include <xdp/libxdp.h>

#include "xdp_program.hxx"
#include "common.h"
#include "utils.hxx"

XdpProgram::XdpProgram(xdp_program* pointer, int if_index)
  : _program(pointer)
  , _if_index(if_index)
{}

XdpProgram::~XdpProgram()
{
  xdp_program__detach(_program, _if_index, _mode, 0);
  xdp_program__close(_program);
  print("The XDP program was released");
}

void
XdpProgram::attach()
{
  auto error = xdp_program__attach(_program, _if_index, _mode, 0);
  if (error) {
    throw std::runtime_error(std::format("Can't attach XDP program to interface. Err code: {}", error));
  }
}

bpf_object*
XdpProgram::bpf_object_ptr()
{
  return xdp_program__bpf_obj(_program);
}

void
XdpProgram::update_map(const std::string& map_name, void* value)
{
  auto map_fd = find_map_fd(map_name);
  auto error = bpf_map_update_elem(map_fd.descriptor, &KEY, value, 0);
  if (error < 0) {
    throw std::runtime_error("Can't update a BPF map");
  }
}

XdpProgram
XdpProgram::from_bpf_obj(bpf_object* object, int if_index)
{
  auto handle = xdp_program__from_bpf_obj(object, "xdp");
  auto error = libxdp_get_error(handle);
  if (error) {
    throw std::runtime_error("Can't create XDP program");
  }
  return {handle, if_index};
}

BpfFileDescriptor
XdpProgram::find_map_fd(const std::string& map_name)
{
  auto bpf_obj = xdp_program__bpf_obj(_program);
	auto map = bpf_object__find_map_by_name(bpf_obj, map_name.c_str());
  if (!map) {
    throw std::runtime_error(std::format("Can't find a map by name: {}", map_name));
	}
	auto file_descriptor = bpf_map__fd(map);
  if (file_descriptor < 0) {
    throw std::runtime_error(std::format("Can't get a file descriptor of a map: {}", map_name));
  }
  return { file_descriptor };
}
