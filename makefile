
.PHONY: all linux linux-x86 win64 win32 clean

# All Target
all: linux linux-x86 win64 win32

linux:
	(cd linux_x64; make)

linux-x86:
	(cd linux_x86; make)

win64:
	(cd win64; make)

win32:
	(cd win32; make)

clean:
	(cd linux_x64; rm prv2pjdump*; make clean)
	(cd linux_x86; rm prv2pjdump*; make clean)
	(cd win64; rm *.exe*; make clean)
	(cd win32; rm *.exe*; make clean)

