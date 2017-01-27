# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

include ../../config.mk

PARSE=../../parse.js

TEMPLATE_DIR=../../templates
TEMPLATES=$(shell find $(TEMPLATE_DIR) -type f)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

$(OUTPUT_DIR)/$(IMPL_PREFIX)%.h: $(INPUT_DIR)/%.idl $(PARSE) $(TEMPLATES) $(OUTPUT_DIR) ../_clang-format
	@$(eval CMD = $(BIN_NODEJS) $(PARSE) $< $(TEMPLATE_DIR) $(OUTPUT_DIR) $(IMPL_PREFIX))
	@$(eval CREATED = $(shell $(CMD)))
	@echo "$(CMD)"
	$(BIN_CLANGFORMAT) -style=file -i $(CREATED)

.PHONY: clean

clean:
	@echo "cleaning"
	@rm -rf $(shell find $(OUTPUT_DIR)/ -type f)
