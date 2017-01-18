INPUT_DIR=./idl
OUTPUT_DIR=./cpp
IMPL_PREFIX=sm_JS_

INPUT_FILES = $(shell find $(INPUT_DIR) -type f -name '*.idl')

TARGETS = $(addprefix \
			$(join $(OUTPUT_DIR)/, $(IMPL_PREFIX)), \
			$(addsuffix .h,  $(basename \
								$(notdir $(INPUT_FILES) ) \
							) \
			) \
		)

all: $(TARGETS)
	@echo $(TARGETS)

include ../common.mk

