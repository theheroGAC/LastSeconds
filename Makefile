TARGET  = lastseconds
OBJS    = main.o
#For use debug, add -lSceNet_stub -lSceNetCtl_stub
LIBS    = -ltaihen_stub
PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -g -Wl,-q -Wall -O3 -nostartfiles
ASFLAGS = $(CFLAGS)
TYPE = suprx
PSVITAIP = 192.168.1.27

all: $(TARGET).$(TYPE)

%.$(TYPE): %.velf
	vita-make-fself $< $@

%.velf: %.elf
	vita-elf-create -n -e exports.yml $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).$(TYPE) $(TARGET).velf $(TARGET).elf $(OBJS)

send: $(TARGET).$(TYPE)
	curl -T $(TARGET).$(TYPE) ftp://$(PSVITAIP):1337/ur0:/tai/$(TARGET).$(TYPE)
	@echo "Plugin sended."
