TARGET := v4l2_test_ser
CC := arm-linux-gcc


CSRCS := $(wildcard *.c)

INCS := -I/home/china/arm-libs/include 


LIBS := -L/home/china/arm-libs/lib -ljpeg -lpthread


  

OBJS := $(patsubst %.c, %.o, $(CSRCS))

$(TARGET): $(OBJS) 
	$(CC) $+ $(INCS) $(LIBS) -o $@ -g
	cp $(TARGET) ~/tftp_share

%.o:%.c
	#必须使用$^才能表示所有的依赖文件，使用$<只能表示第一个依赖文件；
	$(CC) -c $^ $(INCS) -o $@		

clean:
	rm -rf $(OBJS) $(TARGET)
