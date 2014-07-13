test1:  nop 0, 0
test2:  load 0x228, r[7]
test3:  loadp 0x228, r[7]












        


test4:
        store 0x228, r[7]       ;comment
        .data 0xdeadbeef
        .data defined
test5:  storep 0x228, r[7]
test6:  push r[5], r[255]
        .def defined, 0x25
test7:  pop r[5], r[255]
test19: and 8, r[12], r[9]
t101:   and r[12], dined, r[28]
t102:   not r[18], r[18]
test8:  copy r[8], r[255]       ;some more ocmments
        .locate 0x1000
test9:  const 0x2000, r[7]
test10: add r[1], r[2], r[7]    ;yet more comments
test11: sub 19, r[2], r[7]
test12: jump test2
test13: jump 0x223
test17: jump r[defined]
test14: jumpt 0x223, r[7]
test15: call test9, r[255]
test16: call r[8], r[255]
        fork 0x223, 0x108


        ;; comment



        