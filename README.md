# 🔨 nodeclone, a hobby project

A minimal Node.js-like runtime built in **Common Lisp**, using:

* 🧠 [QuickJS](https://bellard.org/quickjs/) — JS engine
* ⚙️ [libuv](https://libuv.org/) — async I/O
* 🧵 CFFI — Lisp ↔ C bridge

Supports:

* `setTimeout(...)` with closures
* `console.log(...)` from JS
* Lisp-driven event loop

---

## 💠 Build & Run

```bash
# Install libuv if missing:
sudo apt install libuv1-dev

# Build native library
make

# Run a JS script
make run FILE=test.js
```

---

## 📁 Structure

```
quickjs/       # QuickJS source
c/             # C wrappers
nodeclone.lisp # Lisp FFI and entry point
Makefile       # Build + run targets
test.js        # Example JS file
```
