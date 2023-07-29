GCC = gcc

FLAGS = -Wall -Wextra

TARGET = serverM serverC serverEE serverCS client

all: $(TARGET)

$(TARGET): serverM.c serverC.c serverEE.c serverCS.c client.c head.h
	$(GCC) $(FLAGS) -o serverM serverM.c
	$(GCC) $(FLAGS) -o serverC serverC.c
	$(GCC) $(FLAGS) -o serverEE serverEE.c
	$(GCC) $(FLAGS) -o serverCS serverCS.c
	$(GCC) $(FLAGS) -o client client.c

clean:
	rm -rf serverM serverC serverEE serverCS client
