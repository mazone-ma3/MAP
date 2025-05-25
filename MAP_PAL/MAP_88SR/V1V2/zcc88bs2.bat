zcc +pc88 -compiler sdcc -DNODELAY -lm -subtype=BASIC -zorg=0xb400 %1.c -o %1.bin %2
z88dk-appmake +msx -b %1.bin --org 0xb400 -o %1.tmp
conv %1.tmp %1.b88
