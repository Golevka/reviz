# INCLUDE_PATH, SOURCE_PATH, DEPENDENCY_PATH, OBJECT_PATH, EXTERNAL_LIBS and
# PROGRAM_NAME should be defined in custom makefile

vpath %.h $(INCLUDE_PATH)
vpath %.c $(SOURCE_PATH)
vpath %.d $(DEPENDENCY_PATH)
vpath %.o $(OBJECT_PATH)


## default .o and .dep path and program name
OBJECT_PATH     ?= obj
DEPENDENCY_PATH ?= dep
PROGRAM_NAME    ?= run

# source trunk
source-files    = $(wildcard  $(addsuffix /*.c, $(SOURCE_PATH)))
source-list     = $(notdir  $(source-files))

# binary trunk
objname-list    = $(subst  .c,.o, $(source-list))
object-list     = $(addprefix  $(OBJECT_PATH)/, $(objname-list))

# dependency trunk
depname-list    = $(subst  .c,.d, $(source-list))
dependency-list = $(addprefix  $(DEPENDENCY_PATH)/, $(depname-list))

# -I option to help the compiler finding the headers
CFLAGS += $(addprefix  -I, $(INCLUDE_PATH))


# Build external library cmdline parameter, those -Xlinker directives instructs
# the linker find the correct linking sequence regardless the order of items
# specified in EXTERNAL_LIBS.
LOADLIBES += \
	-Xlinker --start-group \
		$(addprefix  -Xlinker , $(EXTERNAL_LIBS)) \
	-Xlinker --end-group

# PROGRAM_NAME is provided in custom makefile
$(PROGRAM_NAME): $(object-list)
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@


$(OBJECT_PATH)/%.o: %.c
	@mkdir -p $(OBJECT_PATH)
	$(COMPILE.c) $(OUTPUT_OPTION) $<


# Resolve [object,source] -- [header] dependency
-include $(dependency-list)

$(DEPENDENCY_PATH)/%.d: %.c
	@mkdir -p $(DEPENDENCY_PATH)
	@$(CC) -M $(CFLAGS) $< > $@.$$$$;			\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	rm -f $@.$$$$


.PHONY: clean build
clean:
	rm -f $(object-list) $(dependency-list)
	rm *.dot
