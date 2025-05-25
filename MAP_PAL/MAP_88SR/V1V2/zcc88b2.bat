zcc +pc88 -DNODELAY -lm -subtype=BASIC %1.c -zorg=0xb400 -o %1.bin %2
z88dk-appmake +msx -b %1.bin --org 0xb400 -o %1.tmp
conv %1.tmp %1.b88
