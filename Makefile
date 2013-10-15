#
#
#

TARGET=freertos-demo

# toolchain

CROSS_COMPILE ?= /home/matsi/devel/tools/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux-2013.05-24/bin/arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

# project files

SRC_DIR = src
OBJ_DIR = build
INC_DIR = include

# vendor files

STM_DIR ?= /home/matsi/src/stm32/STM32F4-Discovery_FW_V1.1.0/Libraries
FREERTOS_DIR ?= /home/matsi/code/FreeRTOSV7.3.0/FreeRTOS

# flags

CFLAGS = -g -Wall -O2
CFLAGS += -mcpu=cortex-m4 -mthumb -mthumb-interwork
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS += -I$(INC_DIR)

# vendor settings

CFLAGS += -I$(FREERTOS_DIR)/Source/include -I$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM4F

CFLAGS += -I$(STM_DIR)/STM32F4xx_StdPeriph_Driver/inc -I$(STM_DIR)/CMSIS/ST/STM32F4xx/Include -I$(STM_DIR)/CMSIS/Include
CFLAGS += -DUSE_STDPERIPH_DRIVER

#

LDFLAGS = -Tstm32f4-flash.ld
LIBGCC  = $(shell $(CC) -mthumb -mcpu=cortex-m4 -print-libgcc-file-name)

# project sources

DEMO_SRCS += \
	src/startup_stm32f4xx.s \
	src/system_stm32f4xx.c \
	src/stdlib.c \
	src/main.c

# vendor sources

DEMO_SRCS += \
	$(STM_DIR)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	$(STM_DIR)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c

DEMO_SRCS += \
	$(FREERTOS_DIR)/Source/list.c \
	$(FREERTOS_DIR)/Source/queue.c \
	$(FREERTOS_DIR)/Source/tasks.c \
	$(FREERTOS_DIR)/Source/timers.c \
	$(FREERTOS_DIR)/Source/croutine.c \
	$(FREERTOS_DIR)/Source/portable/MemMang/heap_1.c \
	$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM4F/port.c

#

DEMO_OBJS := $(DEMO_SRCS:.c=.o)
DEMO_OBJS := $(DEMO_OBJS:.s=.o)

DEMO_OBJS := $(addprefix $(OBJ_DIR)/,$(DEMO_OBJS))

# rules

all: $(OBJ_DIR)/$(TARGET).elf $(OBJ_DIR)/$(TARGET).hex $(OBJ_DIR)/$(TARGET).bin

$(OBJ_DIR)/$(TARGET).elf: $(DEMO_OBJS) $(LIBGCC) stm32f4-flash.ld
	$(LD) $(LDFLAGS) $(DEMO_OBJS) $(LIBGCC) -o $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -rf $(OBJ_DIR)

.PHONY: clean
