IBLIS - Important Business Logic Instruction Set
========================================

THREADS and SEGMENTS
--------

A IBLIS CPU is capable of executing a large number of simultaneous
threads of execution. The CPU executes one instruction in each thread
in turn, round robin fashion.

Each thread is restricted to a single memory segment selected when the
thread is launched. Each segment is a complete IBLIS address
space. [Possible restriction: Less than complete amount of memory.]

REGISTERS
---------

Each IBLIS thread has 256 registers, of which 254 are general
purpose. They are addressed by their index, starting at 0 and written
as "r[n]" where n is the register index.

Register 0 is the instruction pointer for the thread. Throughout the
execution of an instruction, r[0] will contain the address of the
currenly-executing instruction.

[Possible expansion: Register 255 is the error trap register.  See "error handling".]

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

ERROR HANDLING
--------------

An error occurs when a thread attempts to execute an unknown or
illegal instruction or when a legal instruction attempts to access a
memory location that is outside its segment.

ADDRESSING MODES
----------------

IBLIS is a RISC-style load/store architecture. Memory may only be
accessed via LOAD, STORE, and stack operations. There are two address
modes for LOAD and STORE:

    DIRECT - the 18-bit address to load/store is directly encoded in
        the instruction.

    INDIRECT - a register is indicated, and its contents are treated
        as the target address for the load/store operation.

Arithmetic and comparison instructions operate only on register
contents or signed 8-bit literal values. Both are directly encoded
into the instruction. There are no indirect-mode arithmetic
operations.

STACK OPERATIONS
----------------

As a nod to modern fashion, IBLIS supports structured programming by
exposing stack-oriented memory operations.

Any register may be used as a stack pointer. In fact, using multiple
registers, it's possible to have mulitple, separate stacks.

SUBROUTINES
-----------

A CALL instruction is provided for subroutine jumps. This stores a
return address on the stack, and then jumps to the argument
address.

No RETURN instruction is provided. For return values, choose a
register to use for return values. To return, use "POP r[0],
stackPointer".

IBLIS does not define any convention for subroutine arguments or
parameters. It is up to each programmer to decide those for
themselves. One possible scheme is to pass parameters on the stack,
and use a particular register as return value. You'll also need to
decide which registers subroutines may freely use, and which they must
leave alone.

For example:

    ; r[128] is the stack pointer
    ; r[127] is return register
    ; r[129] and above are free for subroutine use.
    CONST 125, r[14]
    PUSH r[14], r[128] ; push parameter
    CALL add5, r[128] ; call add5 subroutine
    ADD 1, r[128], r[128] ; clean up parameter
    ;bunch of other code
    add5:
    ADD 1, r[128], r[129] ; get address of parameter
    LOAD r[127], r[129] ; get the parameter
    ADD r[127], 5, r[127] ; retval = retval + 5
    POP r[0], r[128] ; pop the return value into the IP

INSTRUCTION TYPES
-----------------

There are three instruction types, varying slightly in their
encoding. In the following table, "\<n\>" means a sequence of n reserved
or unused bits.

    Type A - operation:mode:address:regC
    Type B - operation:mode:<10>:regB:regC
    Type C - operation:modeA:modeB:<1>:regA:regB:regC

INSTRUCTION SET
---------------

NOP : does nothing, advances IP

LOAD - Load the contents of memory into a register.

    (addr, regC) : stores the contents of addr in regC.
    (regB, regC) : stores the contents of [regB] in regC.
    
LOAD PEER - Load the contents of peer memory into a register.

    (addr, regC) : stores the contents of addr on PEER machine to regC.
    (regB, regC) : stores the contents of [regB] on PEER machine to regC.

STORE - Store the contents of a register into memory.

    (addr, regC) : stores the contents of regC in addr.
    (regB, regC) : stores the contents of regC in [regB].

STORE PEER - Store the contents of a register into peer memory.

    (addr, regC) : stores the contents of regC in addr on PEER machine.
    (regB, regC) : stores the contents of regC in [regB] on PEER machine.

PUSH - Push a word to the stack, using any register as a stack
pointer.

    (regb, regc) : decrements regC, then stores the contents of regB in [regC]
                   (regC is the stack pointer.)

POP - Pop a word from the stack, using any register as a stack
pointer.

    (regB, regC) : loads the contents of [regC] in regB, then
                   increments regC. (regC is the stack pointer.)

COPY - Copy the contents of one register to another.

    (regB, regC) : stores the contents of regC in regB.

CONST - Load a literal value into a register.

    (literal, regC) : stores the signed 18-bit literal in regC.


[ADD, SUB, MUL, DIV, MOD] - Arithemetic operators.

    (a, b, regC) : a or b may be either registers or 8-bit signed
    literals, performs the operation "regC = a . b" where '.' is
    whichever operation is specified.

[CEQ, CL, CLE, CG, CGE] - Comparison operators.

    (a, b, regC) : a or b may be either registers or 8-bit signed
    literals, performs the operation "regC = (a . b) ? 1 : 0".

JUMP - Unconditional jump.

    (addr) : jump to the given address literal
    (regC) : jump to the address stored in regC.

JUMP IF TRUE - Conditional jump.

    (addr, regC) : jump to the given address literal if regC != 0
    (regB, regC) : jump to the address stored in regB if regC != 0

CALL - Push IP to the stack, and jump.

    (addr, regC) : PUSH (r[0] + 1), regC; and jump to addr.
    (regB, regC) : PUSH (r[0] + 1), regC; and jump to address in regB.

RETURN - Return from a CALL instruction.

    Unnecessary. Use: POP r[0], regC;

FORK -

    (addr, segment) : spawn a new thread in the given local segment,
    with initial instruction pointer value equal to addr.



