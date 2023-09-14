.orig x3000
lea r5, address ;r5 should have #2
ldb r3, r5, #0 ;should have 0x0002
ldb r4, r5, #1 ;should have 0x0030
and r6, r3, r4 ;should have 0x0032
finish trap x25
address .fill x3FFF
.end