#pragma once
#include <string>
#include <xdp/libxdp.h>

#include "common.h"

class XdpProgram {
private:
  xdp_program* _program;
  int _if_index = 0;
  xdp_attach_mode _mode = XDP_MODE_UNSPEC;

  /**
   * Use the factory method (from_bpf_obj) instead.
   */
  XdpProgram(xdp_program* pointer, int if_index);

public:
  XdpProgram(const XdpProgram&) = delete;
  XdpProgram(XdpProgram&&) = delete;
  XdpProgram& operator=(const XdpProgram&) = delete;
  XdpProgram& operator=(XdpProgram&&) = delete;

  /**
   * Detaches the XDP program from the network interface.
   */
  ~XdpProgram();

  /**
   * Attaches an XDP program to the network interface that
   * was passed during creation of this XdpProgram.
   * Throws an error on failure.
   */
  void attach();

  /**
   * Returns a BPF object.
   * @return A BPF object pointer.
   */
  bpf_object* bpf_object_ptr();

  /**
   * Updates a BPF map by its name.
   * Throws an error on failure.
   * @param map_name A BPF map name that defined in the kernel part.
   * @param value The value must be compatible with
   *              the corresponding map in the kernel part.
   */
  void update_map(const std::string& map_name, void* value);

  /**
   * Returns a BPF file descriptor based on a BPF map name.
   *
   * @param map_name A BPF map name that defined in the kernel part.
   * @return Returns a file descriptor of the requested map or throws an error.
   */
  BpfFileDescriptor find_map_fd(const std::string& map_name);

  /**
   * A factory method for creating an XDP program based on a BPF object.
   * Do not attach BPF program directly, it shoud be done by the
   * XdpProgram::attach method.
   *
   * @param object A BPF object.
   * @param if_index A network interface index, you may find it
   *                 by running "ip address" command
   * @return Returns a wrapper around the XDP program or throws an error.
   */
  static XdpProgram from_bpf_obj(bpf_object* object, int if_index);
};
