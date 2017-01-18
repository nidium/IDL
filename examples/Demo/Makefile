INPUT_DIR=./idl
OUTPUT_DIR=./cpp

INPUT_FILES = $(shell find $(INPUT_DIR) -type f -name '*.idl')

TARGETS = $(addprefix \
			$(join $(OUTPUT_DIR)/, base_), \
			$(addsuffix .h,  $(basename \
								$(notdir $(INPUT_FILES) ) \
							) \
			) \
		)

all: $(TARGETS)

include ../common.mk

