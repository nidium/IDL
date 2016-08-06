NODE=nodejs
NPM=npm
CLANGFORMAT=clang-format-3.5 -style=Mozilla

PROJECT=./examples/APE.idl
OUTPUT_DIR=./output/APE/
TEMPLATE_DIR=./templates/
TEMPLATES=$(shell find $(TEMPLATE_DIR) -type f)

TARGET=$(OUTPUT_DIR)/base_IDLTest.h

all: $(TARGET)

setup: package.json
	@$(NPM) install

$(TARGET): parse.js $(PROJECT) $(TEMPLATES)
	@echo "creating"
	@$(NODE) parse.js $(PROJECT) $(TEMPLATE_DIR) $(OUTPUT_DIR)
	@echo "formatting"
	@$(CLANGFORMAT) -i $(shell find $(OUTPUT_DIR) -type f )


.PHONY: clean

clean:
	@echo "cleaning"
	@rm -rf $(shell find $(OUTPUT_DIR) -type f -a \( -name '*.c' -o -name '*.cpp' -o -name '*.h' \))
