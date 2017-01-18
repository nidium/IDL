include ../../config.mk

PARSE=../../parse.js

TEMPLATE_DIR=../../templates
TEMPLATES=$(shell find $(TEMPLATE_DIR) -type f)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

$(TARGET): $(PARSE) $(PROJECT) $(TEMPLATES) $(OUTPUT_DIR) ../_clang-format
	@echo "creating $@"
	@$(BIN_NODEJS) $(PARSE) $(PROJECT) $(TEMPLATE_DIR) $(OUTPUT_DIR)
	@echo "formatting $@"
	@$(BIN_CLANGFORMAT) -style=file -i $(OUTPUT_DIR)/*

.PHONY: clean

clean:
	@echo "cleaning"
	@rm -rf $(shell find $(OUTPUT_DIR)/ -type f)
