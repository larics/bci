

	    Basic CAN Interface library for Linux


To build the library just run 'make' in bci directory.

After that, you will have a library .so file, which you have to install to
your system (use 'make install'). 

There are demo programs (demo.c and singledemo.c), which illustrate some
basic features of the BCI library. (See source code and comments).
To build them run 'make demo'

NOTE:

1) For compiling your application with BCI library you have to specify 
   -lBCI linker key (see Makefile, demo section).  
2) Use CAN boards with two CAN controllers.   
3) To run 'demo' program , you have to connect together both CAN interfaces
   on your card. Don't forget about terminator.
4) 'singledemo' uses only first controller on CAN board. It sends some messages
   (MESSAGES_NUMBER) to the CAN bus and then waits for incomming messages.



