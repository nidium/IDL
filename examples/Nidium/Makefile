# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

INPUT_DIR=./idl
OUTPUT_DIR=./cpp
IMPL_PREFIX=sm_JS_

DOC_DIR = ../../..Nidium/docs
DOC2IDL=./../../NativeTools/IDL/doc2idl_transformer.py

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

$(OUTPUT_DIR)/$(IMPL_PREFIX)%.h: $(DOC_DIR)/%.cpp.py $(DOC2IDL)
	$(DOC2IDL) $< $@

include ../common.mk



