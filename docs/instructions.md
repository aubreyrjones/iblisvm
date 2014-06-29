IBLIS - Important Business Logic Instruction Set
========================================

THREADS and SEGMENTS
--------

Each IBLIS CPU is capable of executing a large number of simultaneous
threads of execution.

However, each thread is restricted to a single memory segment selected
when the thread is launched. Each segment is a complete IBLIS address
space.

REGISTERS
---------

Each IBLIS thread has 256 registers, of which 255 are general
purpose. They are addressed by their index, starting at 0. Register 0
is the instruction pointer for the thread.

All registers are 32 bits. For all arithmetic and comparison
operations, the contents of the registers are treated as SIGNED
numbers.

MEMORY
------

IBLIS defines an 18-bit address space. This allows referencing up to
262,144 words of storage. This should be enough for any purpose.

When addressing memory indirectly through a register, 32-bit
arithmetic words are treated as unsigned, but are not otherwise
converted. If the word encodes a value which would address a word
beyond the IBLIS address space, an exception occurs and the current
thread is halted.

COMMUNICATIONS
--------------

In addition to the thread's local memory segment, a thread may also be
connected to a single remote segment, called the peer segment.

ADDRESSING MODES
-------------

IBLIS is a RISC-style load/store architecture. Memory may only be
accessed via LOAD and STORE operations. There are two address modes
for LOAD and STORE:

    DIRECT - the 18-bit address to load/store is directly encoded in
        the instruction.

    INDIRECT - a register is indicated, and its contents are treated
        as the target address for the load/store operation.

Arithmetic and comparison instructions operate only on register
contents or signed 8-bit literal values. Both are directly encoded
into the instruction. There are no indirect-mode arithmetic
operations.

INSTRUCTION TYPES
-----------------

There are three instruction types, varying slightly in their
encoding. In the following table, "<>" means a sequence of 1 or more
reserved or unused bits.

Type A - operation:mode:address:regC
Type B - operation:mode:<>:regB:regC
Type C - operation:modeA:modeB:<>:regA:regB:regC

INSTRUCTION SET
---------------

NOP : does nothing, advances IP

LOAD -
    (addr, regC) : stores the contents of addr in regC.
    (regB, regC) : stores the contents of [regB] in regC.
    
LOAD PEER -
    (addr, regC) : stores the contents of addr on PEER machine to regC.
    (regB, regC) : stores the contents of [regB] on PEER machine to regC.

STORE -
    (addr, regC) : stores the contents of regC in addr.
    (regB, regC) : stores the contents of regC in [regB].

STORE PEER -
    (addr, regC) : stores the contents of regC in addr on PEER machine.
    (regB, regC) : stores the contents of regC in [regB] on PEER machine.

COPY -
    (regB, regC) : stores the contents of regC in regB.

CONST -
    (literal, regC) : stores the signed 18-bit literal in regC.


[ADD, SUB, MUL, DIV, MOD] -
    (a, b, regC) : a or b may be either registers or 8-bit signed
    literals, performs the operation "regC = a . b" where '.' is
    whichever operation is specified.

[CEQ, CL, CLE, CG, CGE] -
    (a, b, regC) : a or b may be either registers or 8-bit signed
    literals, performs the operation "regC = (a . b) ? 1 : 0".


JUMP -
    (addr) : jump to the given address literal
    (regC) : jump to the address stored in regC.

JUMP IF TRUE -
    (addr, regC) : jump to the given address literal if regC != 0
    (regB, regC) : jump to the address stored in regB if regC != 0


FORK -
    (segment, addr) : spawn a new thread in the given local segment,
    with initial instruction pointer value equal to addr.


