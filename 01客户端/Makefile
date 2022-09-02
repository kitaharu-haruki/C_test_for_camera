

TARGET := v4l2_test_cli
CC := arm-linux-gcc


CSRCS := $(wildcard *.c)

INCS := -I/home/china/arm-libs/include 


LIBS := -L/home/china/arm-libs/lib -ljpeg -lpthread




OBJS := $(patsubst %.c, %.o, $(CSRCS))

$(TARGET): $(OBJS) 
	$(CC) $+ $(INCS) $(LIBS) -o $@ -g
	cp $(TARGET) ~/tftp_share

%.o:%.c
	$(CC) -c $< $(INCS) -o $@


clean:
	rm -rf $(OBJS) $(TARGET)
