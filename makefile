compiler := gcc

# FLAGS := -DTARGET=0

SSD ?= 1
PCC ?= 1
INCLUDE = include
FLAGS := -DSSD_TEST=$(SSD) -DPCC_TEST=$(PCC)
suffix_FLAGS := -lpthread -lm


build: main.c ${INCLUDE}/PCC.h ${INCLUDE}/SSD.h 
	$(compiler) $(FLAGS) main.c -o main.out $(suffix_FLAGS)

run: main.out
	bash ./test.sh $(TARGET)

PCC:
	make build PCC=1 SSD=0
	make run TARGET=$(TARGET)

SSD:
	make build PCC=0 SSD=1
	make run TARGET=$(TARGET)

main.out: 
	build 

all : main.out
	make run TARGET=$(TARGET)
	
clean:
	rm main.out