NODE=nodejs
NPM=npm
CLANGFORMAT=clang-format-3.5 -style=Mozilla

PROJECT=./examples/Hello.idl
OUTPUT_DIR=./output/Hello
TEMPLATE_DIR=./templates

TEMPLATES=$(shell find $(TEMPLATE_DIR) -type f)
TARGET=$(OUTPUT_DIR)/base_Hello.h

all: $(TARGET)

setup: package.json
	@$(NPM) install

$(TARGET): parse.js $(PROJECT) $(TEMPLATES)
	@echo "creating"
	@$(NODE) parse.js $(PROJECT) $(TEMPLATE_DIR) $(OUTPUT_DIR)
	@echo "formatting"
	@$(CLANGFORMAT) -i $(OUTPUT_DIR)/*


.PHONY: clean

clean:
	@echo "cleaning"
	@rm -rf $(shell find $(OUTPUT_DIR)/ -type f)
