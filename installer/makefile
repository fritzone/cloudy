#
# The DOS installer of cloudy, plain C in the small memory model
#

install.exe : install.c .AUTODEPEND
  wcl -q -0 -ms -os -k8192 -bt=dos -fe=install.exe install.c

clean : .symbolic
  @rm -f install.exe *.o *.obj *.err
