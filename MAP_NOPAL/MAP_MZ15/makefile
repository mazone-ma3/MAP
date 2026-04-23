# --- 設定 ---
TARGET   = MAPMZ15
CC       = zcc

# MZ-1500用フラグ (SDCC, 速度優先, 引数渡しをsdcccall0に固定)
CFLAGS   = +mz -compiler sdcc --sdcccall0 --opt-code-speed
LDFLAGS  = +mz -create-app -lm

# 中間ファイル
OBJS     = $(TARGET).o

# --- ルール ---

all: $(TARGET).com

# 1. パターンルール
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# 2. メインターゲット (D88ファイル生成)
$(TARGET).com: $(OBJS)
	$(CC) +cpm -create-app -o $(TARGET).COM $(OBJS) -lm -m

# 依存関係
$(TARGET).o: $(TARGET).c inkey.h

# 3. おまけ：d88版をテストしたい場合(未対応)
#cpm
d88: $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) -m -v

clean:
	rm -f *.o *.d88 *.map *.COM *.ihx
