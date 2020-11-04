#include <nusys.h>

beginseg
    name    "code"
    flags   BOOT OBJECT
    entry   nuBoot
    address 0x80000400
    stack   NU_SPEC_BOOT_STACK
    include "codesegment.o"
	include "$(ROOT)/usr/lib/PR/rspboot.o"
	include "$(ROOT)/usr/lib/PR/aspMain.o"
	include "$(ROOT)/usr/lib/PR/n_aspMain.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspL3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.Rej.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DLX2.Rej.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspS2DEX2.fifo.o"
endseg

beginseg
	name "file_pack"
	flags RAW
	include "file_pack.bin"
endseg

beginwave
    name    "compiledrom"
    include "code"
	include "file_pack"
endwave