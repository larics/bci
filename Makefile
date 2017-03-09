CC=gcc

CC_FLAGS := -Wall -O2 -DLINUX -I. -fno-strict-aliasing
CC_SO_FLAGS := -Wall -O2 -DLINUX -fPIC -I. -fno-strict-aliasing

CC_SO_FLAGS += -DBCI_DEBUG=0 -DBCI_IPCI_DEBUG=0

# CC_SO_FLAGS += -DBCI_TEST
CC_SO_FLAGS += -ggdb

MAJOR:=4
MINOR:=5
PROJECT_DIR := ./
LIB_DIR := /usr/lib
CAN_DEV_PATH := /dev
CAN_MAJOR0 := 248                                                 #modified 31/03/2015  D.M.
CAN_MAJOR1 := 251
INCLUDES := bci.h bci_int.h dpram.h integral.h ipci.h
BCI_OBJ := bci.o dpram.o ipci.o pc_i.o bapfw165.o bapfw320.o bapfw161.o filterlist.o ipci_lin.o bci_lin.o

all: libBCI

libBCI: clean $(BCI_OBJ)
				$(CC) -shared -Wl,-soname,libBCI.so.$(MAJOR) -o libBCI.so.$(MAJOR).$(MINOR) $(BCI_OBJ)

demo: singledemo.c demo.c lindemo.c Makefile
				cd $(PROJECT_DIR)
				$(CC) $(CC_FLAGS) -o demo demo.c -L$(LIB_DIR) -lBCI
				$(CC) $(CC_FLAGS) -o singledemo singledemo.c -L$(LIB_DIR) -lBCI
				$(CC) $(CC_FLAGS) -o lindemo lindemo.c  -L./ -L$(LIB_DIR) -lBCI

floader: floader.c Makefile
				cd $(PROJECT_DIR)
				$(CC) $(CC_FLAGS) -o floader floader.c -L$(LIB_DIR) -lBCI

test: bcitest.c bcilongtest.c busmon.c
				$(CC) $(CC_FLAGS) -lBCI -o bcitest bcitest.c
				$(CC) $(CC_FLAGS) -lBCI -o bcilongtest bcilongtest.c
				$(CC) $(CC_FLAGS) -lBCI -o busmon busmon.c
				$(CC) $(CC_FLAGS) -lBCI -lpopt -o bciperf bciperf.c

%.o : %.c $(INCLUDES)
				$(CC) $(CC_SO_FLAGS) -c $< -o $@


install: libBCI nodes
				sudo rm -f $(LIB_DIR)/libBCI.*
				sudo cp libBCI.so.$(MAJOR).$(MINOR) $(LIB_DIR)/
				sudo ln -s $(LIB_DIR)/libBCI.so.$(MAJOR).$(MINOR) $(LIB_DIR)/libBCI.so.$(MAJOR)
				sudo ln -s $(LIB_DIR)/libBCI.so.$(MAJOR).$(MINOR) $(LIB_DIR)/libBCI.so
				sudo /sbin/ldconfig -nv /usr/lib | grep BCI

reinstall: clean install test demo

rebuild: clean all

nodes:
	sudo rm -f $(CAN_DEV_PATH)/can0
	sudo rm -f $(CAN_DEV_PATH)/can1
	sudo rm -f $(CAN_DEV_PATH)/can2
	sudo rm -f $(CAN_DEV_PATH)/can3
	if [ ! -c $(CAN_DEV_PATH)/can0 ]; then sudo mknod $(CAN_DEV_PATH)/can0 c $(CAN_MAJOR0) 0; fi
	if [ ! -c $(CAN_DEV_PATH)/can1 ]; then sudo mknod $(CAN_DEV_PATH)/can1 c $(CAN_MAJOR0) 1; fi
	if [ ! -c $(CAN_DEV_PATH)/can2 ]; then sudo mknod $(CAN_DEV_PATH)/can2 c $(CAN_MAJOR1) 0; fi
	if [ ! -c $(CAN_DEV_PATH)/can3 ]; then sudo mknod $(CAN_DEV_PATH)/can3 c $(CAN_MAJOR1) 1; fi
	sudo chmod a+rw $(CAN_DEV_PATH)/can*

indent:
		indent \
		--brace-indent0 \
		--dont-break-procedure-type \
		--indent-level2 \
		--no-tabs \
		--swallow-optional-blank-lines \
		--blank-lines-after-declarations \
		--blank-lines-after-procedures \
		--blank-lines-before-block-comments \
		--line-length85 \
		--comment-indentation45 \
		--case-brace-indentation2 \
		--case-indentation2 \
		 *.[ch]


clean:
				rm -f *.o
				rm -f *~
				rm -f libBCI*
				rm -f demo singledemo



