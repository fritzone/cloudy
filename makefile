#
# clouder main makefile
#

tcp_h_dir = ../../tcpinc
tcp_c_dir = ../../tcplib
common_h_dir = ../../include
protocol_dir = ./msg_prot
include_dir = ./inc
include_utils_dir = $(include_dir)/utils
include_statemachine_dir = $(include_dir)/states
include_gui_dir = $(include_dir)/gui
include_net_dir = $(include_dir)/net
include_dos_gui_dir = $(include_gui_dir)/dos
include_dos_net_dir = $(include_net_dir)/dos
src_dir = ./src
ext_dir = ./ext
include_ext_dir = $(ext_dir)/inc
include_ezxml_ext_dir = $(include_ext_dir)/ezxml

!include $(protocol_dir)/makefile
!include $(src_dir)/makefile
!include $(ext_dir)/makefile

memory_model = -ml
compile_options = -0 -oh -q -oa -os -s -xs $(memory_model) -DCFG_H=\"cloud.cfg\"
compile_options += -i=$(tcp_h_dir) -i=$(common_h_dir) -i=. -i=$(include_dir) -i=$(protocol_dir) -i=$(include_utils_dir) -i=$(include_gui_dir) -i=$(include_dos_gui_dir) -i=$(include_statemachine_dir) -i=$(include_net_dir) -i=$(include_dos_net_dir) -i=$(include_ezxml_ext_dir)

tcpobjs = packet.o arp.o eth.o ip.o tcp.o tcpsockm.o utils.o udp.o dns.o timer.o ipasm.o trace.o
objs = cloud.o

all : clean cloud.exe

clean : .symbolic
  @rm -f cloud.exe
  @rm -f *.o
  @rm -f *.map

.asm : $(tcp_c_dir)

.cpp : $(tcp_c_dir);$(protocol_dir);$(src_dir);$(utils_dir);$(gui_dir);$(dos_gui_dir);$(states_dir);$(dos_net_dir);$(net_dir)

.c : $(tcp_c_dir);$(ezxml_ext_src_dir)

.asm.o :
  wasm -q -0 $(memory_model) $[*

.cpp.o :
  wpp $[* $(compile_options)

.c.o :
  wpp $[* $(compile_options)

cloud.exe: $(tcpobjs) $(objs) $(protocol_objs) $(util_objs) $(gui_objs) $(dos_gui_objs) $(states_objs) $(ezxml_objs) $(dos_net_objs) $(net_objs) $(src_objs)
  wlink System dos debug watcom all OPtion map OPtion eliminate OPtion stack=4096 Name $@ File {$(tcpobjs) $(objs) $(protocol_objs) $(util_objs) $(gui_objs) $(dos_gui_objs) $(states_objs) $(ezxml_objs) $(dos_net_objs) $(net_objs) $(src_objs)}

