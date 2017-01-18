# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

BIN_CLANGFORMAT := $(shell command -v clang-format-3.5 2> /dev/null)
BIN_NPM := $(shell command -v npm 2> /dev/null)
BIN_NODEJS := $(shell command -v nodejs 2> /dev/null)

ifndef BIN_CLANGFORMAT
    $(error "clang-format-3.5 is not available. Please install")
endif

ifndef BIN_NPM
    $(error "npm is not available. Please install")
endif

ifndef BIN_NODEJS
    $(error "nodejs is not available. Please install")
endif

.PHONY: setup

setup: package.json
	@$(BIN_NPM) install
	@echo "All node dependencies are installed; Have a look at the examples"

