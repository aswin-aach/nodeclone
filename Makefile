# Makefile for nodeclone

LISP=nodeclone.lisp
OUT=libnodeclone.so
QJS_DIR=quickjs
QJS_VERSION=2021-03-27
QJS_URL=https://bellard.org/quickjs/quickjs-$(QJS_VERSION).tar.xz
QJS_TAR=quickjs-$(QJS_VERSION).tar.xz

C_DIR=c
CFILES=$(QJS_DIR)/quickjs.c $(QJS_DIR)/cutils.c $(QJS_DIR)/libregexp.c $(QJS_DIR)/libunicode.c \
       $(C_DIR)/quickjs-wrapper.c $(C_DIR)/libuv-wrapper.c

CFLAGS=-fPIC -shared -I$(QJS_DIR) -luv -DCONFIG_VERSION=\"nodeclone-0.1\"

all: check-deps $(OUT)

$(OUT): $(CFILES)
	@echo "🛠 Building shared library..."
	gcc $(CFLAGS) $(CFILES) -o $(OUT)

run: $(OUT)
	@echo "🚀 Running nodeclone..."
	sbcl --noinform --load $(LISP) --eval "(main)" test.js

clean:
	rm -f $(OUT)
	rm -rf $(QJS_DIR)
	rm -f $(QJS_TAR)

check-deps:
	@echo "🔍 Checking dependencies..."
	@if [ ! -d "$(QJS_DIR)" ]; then \
	  echo "📦 Downloading QuickJS..."; \
	  curl -LO $(QJS_URL); \
	  tar xf $(QJS_TAR); \
	  mv quickjs-$(QJS_VERSION) $(QJS_DIR); \
	fi
	@if ! pkg-config --exists libuv; then \
	  echo "❌ libuv not found! Please install it via your package manager (e.g., sudo apt install libuv1-dev)"; \
	  exit 1; \
	else \
	  echo "✅ libuv found."; \
	fi

.PHONY: all clean run check-deps
